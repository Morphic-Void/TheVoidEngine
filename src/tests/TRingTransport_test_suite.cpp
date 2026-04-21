
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  TRingTransport_test_suite.cpp
//
//  Standalone test suite for threading::transports::TRing<T>.

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "threading/transports/core/TRingTransport.hpp"
#include "tests/TRingTransport_test_suite.hpp"
#include "memory/memory_allocation.hpp"
#include "debug/debug.hpp"

using threading::transports::TRing;

namespace tests
{

struct TTestContext
{
    std::uint32_t passed = 0u;
    std::uint32_t failed = 0u;

    void fail(const char* const expr,
        const char* const file,
        const int line,
        const std::string& message = {},
        const char* const case_name = nullptr)
    {
        ++failed;
        std::cerr << file << '(' << line << "): FAIL: ";
        if ((case_name != nullptr) && (case_name[0] != '\0'))
        {
            std::cerr << '[' << case_name << "] ";
        }
        std::cerr << expr;
        if (!message.empty())
        {
            std::cerr << " : " << message;
        }
        std::cerr << '\n';
    }

    void pass() noexcept
    {
        ++passed;
    }

    [[nodiscard]] int exit_code() const noexcept
    {
        return (failed == 0u) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
};

#define TEST_EXPECT_TRUE(ctx, expr) \
    do { if (expr) { (ctx).pass(); } else { (ctx).fail(#expr, __FILE__, __LINE__); } } while (false)

#define TEST_EXPECT_FALSE(ctx, expr) \
    do { if (!(expr)) { (ctx).pass(); } else { (ctx).fail("!(" #expr ")", __FILE__, __LINE__); } } while (false)

#define TEST_EXPECT_EQ(ctx, lhs, rhs) \
    do { \
        const auto test_lhs_value = (lhs); \
        const auto test_rhs_value = (rhs); \
        if (test_lhs_value == test_rhs_value) { \
            (ctx).pass(); \
        } else { \
            (ctx).fail(#lhs " == " #rhs, __FILE__, __LINE__); \
        } \
    } while (false)

#define TEST_CASE_EXPECT_TRUE(ctx, case_name, expr) \
    do { if (expr) { (ctx).pass(); } else { (ctx).fail(#expr, __FILE__, __LINE__, {}, (case_name)); } } while (false)

#define TEST_CASE_EXPECT_FALSE(ctx, case_name, expr) \
    do { if (!(expr)) { (ctx).pass(); } else { (ctx).fail("!(" #expr ")", __FILE__, __LINE__, {}, (case_name)); } } while (false)

#define TEST_CASE_EXPECT_EQ(ctx, case_name, lhs, rhs) \
    do { \
        const auto test_lhs_value = (lhs); \
        const auto test_rhs_value = (rhs); \
        if (test_lhs_value == test_rhs_value) { \
            (ctx).pass(); \
        } else { \
            (ctx).fail(#lhs " == " #rhs, __FILE__, __LINE__, {}, (case_name)); \
        } \
    } while (false)

inline std::string make_repeated_string(const char c, const std::uint32_t count)
{
    return std::string(static_cast<std::size_t>(count), c);
}

template<typename TTransport>
bool post_string(TTransport& transport, const std::string_view text) noexcept
{
    return transport.post(text.data(), static_cast<std::uint32_t>(text.size()));
}

template<typename TTransport>
bool read_exact_string(TTransport& transport, std::string& out, const std::uint32_t count)
{
    out.resize(static_cast<std::size_t>(count));
    if (!transport.read(out.data(), count))
    {
        out.clear();
        return false;
    }
    return true;
}

template<typename TRing>
std::string drain_ring(TRing& ring)
{
    const std::uint32_t count = ring.readable_count();
    std::string result;
    result.resize(static_cast<std::size_t>(count));
    if ((count != 0u) && !ring.read(result.data(), count))
    {
        return {};
    }
    return result;
}

template<typename TQueue>
std::string drain_queue(TQueue& queue)
{
    std::string result;

    for (;;)
    {
        const std::uint32_t available = queue.refresh_readable_count();
        if (available == 0u)
        {
            break;
        }

        const std::size_t old_size = result.size();
        result.resize(old_size + static_cast<std::size_t>(available));

        if (!queue.read(result.data() + old_size, available))
        {
            return {};
        }
    }

    return result;
}

inline void print_summary(const char* const suite_name, const TTestContext& ctx)
{
    std::cout
        << suite_name
        << ": passed=" << ctx.passed
        << " failed=" << ctx.failed
        << '\n';
}

void test_ring_uninitialised_state(TTestContext& ctx)
{
    TRing<char> ring;

    TEST_EXPECT_TRUE(ctx, ring.is_valid());
    TEST_EXPECT_FALSE(ctx, ring.is_ready());
    TEST_EXPECT_TRUE(ctx, ring.posting_is_valid());
    TEST_EXPECT_TRUE(ctx, ring.reading_is_valid());

    TEST_EXPECT_EQ(ctx, ring.readable_count(), 0u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 0u);

    TEST_EXPECT_TRUE(ctx, ring.post(nullptr, 0u) == false); // not ready
    TEST_EXPECT_TRUE(ctx, ring.read(nullptr, 0u) == false); // not ready
}

void test_ring_initialise_and_conditioning(TTestContext& ctx)
{
    {
        TRing<char> ring;
        TEST_EXPECT_TRUE(ctx, ring.initialise(0u));
        TEST_EXPECT_TRUE(ctx, ring.is_ready());
        TEST_EXPECT_EQ(ctx, ring.readable_count(), 0u);
        TEST_EXPECT_EQ(ctx, ring.writable_count(), TRing<char>::k_min_capacity);
    }

    {
        TRing<char> ring;
        TEST_EXPECT_TRUE(ctx, ring.initialise(33u));
        TEST_EXPECT_EQ(ctx, ring.writable_count(), 64u);
        TEST_EXPECT_FALSE(ctx, ring.initialise(64u));
    }

    {
        TRing<char> ring;
        TEST_EXPECT_FALSE(ctx, ring.initialise(TRing<char>::k_max_capacity + 1u));
        TEST_EXPECT_FALSE(ctx, ring.is_ready());
        TEST_EXPECT_TRUE(ctx, ring.is_valid());
    }
}

void test_ring_initial_allocation_failure(TTestContext& ctx)
{
    TRing<char> ring;

    const bool previous = memory::enable_allocation(false);
    (void)previous;

    TEST_EXPECT_FALSE(ctx, ring.initialise(TRing<char>::k_min_capacity));

    (void)memory::enable_allocation(true);

    TEST_EXPECT_FALSE(ctx, ring.is_ready());
    TEST_EXPECT_TRUE(ctx, ring.is_valid());
}

void test_ring_zero_and_null_behaviour(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));

    TEST_EXPECT_TRUE(ctx, ring.post(nullptr, 0u));
    TEST_EXPECT_TRUE(ctx, ring.read(nullptr, 0u));

    TEST_EXPECT_FALSE(ctx, ring.post(nullptr, 1u));
    TEST_EXPECT_FALSE(ctx, ring.read(nullptr, 1u));
}

void test_ring_basic_single_and_bulk_transfer(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));

    TEST_EXPECT_TRUE(ctx, ring.post('A'));
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 1u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 31u);

    char c = '\0';
    TEST_EXPECT_TRUE(ctx, ring.read(c));
    TEST_EXPECT_EQ(ctx, c, 'A');
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 0u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 32u);

    const std::string text = "HELLO";
    TEST_EXPECT_TRUE(ctx, post_string(ring, text));
    TEST_EXPECT_EQ(ctx, drain_ring(ring), text);
}

void test_ring_full_reject_and_reuse(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));

    const std::string full = make_repeated_string('X', 32u);
    TEST_EXPECT_TRUE(ctx, post_string(ring, full));
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 32u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 0u);

    TEST_EXPECT_FALSE(ctx, ring.post('Y'));

    std::string first_half;
    TEST_EXPECT_TRUE(ctx, read_exact_string(ring, first_half, 16u));
    TEST_EXPECT_EQ(ctx, first_half, make_repeated_string('X', 16u));
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 16u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 16u);

    const std::string refill = "ABCDEFGHIJKLMNOP";
    TEST_EXPECT_TRUE(ctx, post_string(ring, refill));

    const std::string tail = drain_ring(ring);
    TEST_EXPECT_EQ(ctx, tail, make_repeated_string('X', 16u) + refill);
}

void test_ring_wraparound_write_and_read(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));

    TEST_EXPECT_TRUE(ctx, post_string(ring, "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456"));
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 32u);

    std::string prefix;
    TEST_EXPECT_TRUE(ctx, read_exact_string(ring, prefix, 28u));
    TEST_EXPECT_EQ(ctx, prefix, std::string("ABCDEFGHIJKLMNOPQRSTUVWXYZ12"));

    TEST_EXPECT_TRUE(ctx, post_string(ring, "abcd"));
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 8u);

    const std::string suffix = drain_ring(ring);
    TEST_EXPECT_EQ(ctx, suffix, std::string("3456abcd"));
}

void test_ring_repeated_fill_drain_cycles(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));

    for (std::uint32_t i = 0u; i < 8u; ++i)
    {
        const char c = static_cast<char>('A' + static_cast<char>(i));
        const std::string payload = make_repeated_string(c, 24u);

        TEST_EXPECT_TRUE(ctx, post_string(ring, payload));
        TEST_EXPECT_EQ(ctx, drain_ring(ring), payload);
        TEST_EXPECT_EQ(ctx, ring.readable_count(), 0u);
        TEST_EXPECT_EQ(ctx, ring.writable_count(), 32u);
        TEST_EXPECT_TRUE(ctx, ring.is_valid());
        TEST_EXPECT_TRUE(ctx, ring.posting_is_valid());
        TEST_EXPECT_TRUE(ctx, ring.reading_is_valid());
    }
}

void test_ring_deallocate_restores_empty(TTestContext& ctx)
{
    TRing<char> ring;
    TEST_EXPECT_TRUE(ctx, ring.initialise(32u));
    TEST_EXPECT_TRUE(ctx, post_string(ring, "DATA"));

    ring.deallocate();

    TEST_EXPECT_TRUE(ctx, ring.is_valid());
    TEST_EXPECT_FALSE(ctx, ring.is_ready());
    TEST_EXPECT_TRUE(ctx, ring.posting_is_valid());
    TEST_EXPECT_TRUE(ctx, ring.reading_is_valid());
    TEST_EXPECT_EQ(ctx, ring.readable_count(), 0u);
    TEST_EXPECT_EQ(ctx, ring.writable_count(), 0u);
}

int test_ring_transport()
{
    TTestContext ctx;

    test_ring_uninitialised_state(ctx);
    test_ring_initialise_and_conditioning(ctx);
    test_ring_initial_allocation_failure(ctx);
    test_ring_zero_and_null_behaviour(ctx);
    test_ring_basic_single_and_bulk_transfer(ctx);
    test_ring_full_reject_and_reuse(ctx);
    test_ring_wraparound_write_and_read(ctx);
    test_ring_repeated_fill_drain_cycles(ctx);
    test_ring_deallocate_restores_empty(ctx);

    print_summary("TRingTransport", ctx);
    return ctx.exit_code();
}

}   //  namespace tests

int run_ring_transport_tests()
{
    debug_utils::disable_asserts();
    int result = tests::test_ring_transport();
    debug_utils::enable_asserts();
    return result;
}