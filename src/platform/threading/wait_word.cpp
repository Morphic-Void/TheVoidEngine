
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   wait_word.cpp
//  Author: Ritchie Brannan
//  Date:   7 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Low-level atomic wait/wake primitive implementation.

#include <atomic>       //  std::atomic
#include <cstdint>      //  std::uint32_t

#include "platform/threading/wait_word.hpp"
#include "platform/platform_defines.hpp"

#if defined(MV_PLATFORM_WINDOWS)
#include "platform/windows_include.hpp"
#endif

#if defined(MV_PLATFORM_LINUX_ONLY)
#include <limits.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace platform::threading
{

//==============================================================================
//  Internals
//==============================================================================

namespace internal
{

#if defined(MV_PLATFORM_WINDOWS) || defined(MV_PLATFORM_LINUX_ONLY)

static_assert((sizeof(std::atomic<std::uint32_t>) == sizeof(std::uint32_t)),
    "std::atomic<std::uint32_t> must have the same storage size as std::uint32_t.");

static_assert((alignof(std::atomic<std::uint32_t>) >= alignof(std::uint32_t)),
    "std::atomic<std::uint32_t> must be naturally aligned for 32-bit waits.");

static_assert((std::atomic<std::uint32_t>::is_always_lock_free),
    "std::atomic<std::uint32_t> must be always lock-free.");

#endif

#if defined(MV_PLATFORM_WINDOWS)

void* wait_address_from_word(const std::atomic<std::uint32_t>& word) noexcept
{
    const void* const address = static_cast<const void*>(&word);

    return const_cast<void*>(address);
}

#elif defined(MV_PLATFORM_LINUX_ONLY)

const std::uint32_t* futex_address_from_word(const std::atomic<std::uint32_t>& word) noexcept
{
    return reinterpret_cast<const std::uint32_t*>(&word);
}

long futex_wait_private(const std::uint32_t* const address, const std::uint32_t value) noexcept
{
    return ::syscall(
        SYS_futex,
        const_cast<std::uint32_t*>(address),
        FUTEX_WAIT_PRIVATE, value,
        nullptr, nullptr, 0);
}

long futex_wake_private(const std::uint32_t* const address, const int waiter_count) noexcept
{
    return ::syscall(
        SYS_futex,
        const_cast<std::uint32_t*>(address),
        FUTEX_WAKE_PRIVATE, waiter_count,
        nullptr, nullptr, 0);
}

#endif

}   //  namespace internal

//==============================================================================
//  Implementation
//==============================================================================

#if defined(MV_PLATFORM_WINDOWS)

void wait_while_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept
{
    //  WaitOnAddress performs the compare-and-park operation. Failure,
    //  mismatch, and spurious return are normal at this abstraction level.
    (void)WaitOnAddress(
        internal::wait_address_from_word(word),
        static_cast<void*>(const_cast<std::uint32_t*>(&value)),
        sizeof(value), INFINITE);
}

void wake_one_waiter(const std::atomic<std::uint32_t>& word) noexcept
{
    WakeByAddressSingle(internal::wait_address_from_word(word));
}

void wake_all_waiters(const std::atomic<std::uint32_t>& word) noexcept
{
    WakeByAddressAll(internal::wait_address_from_word(word));
}

#elif defined(MV_PLATFORM_LINUX_ONLY)

void wait_while_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept
{
    const std::uint32_t* const address = internal::futex_address_from_word(word);

    (void)internal::futex_wait_private(address, value);
}

void wake_one_waiter(const std::atomic<std::uint32_t>& word) noexcept
{
    const std::uint32_t* const address = internal::futex_address_from_word(word);

    (void)internal::futex_wake_private(address, 1);
}

void wake_all_waiters(const std::atomic<std::uint32_t>& word) noexcept
{
    const std::uint32_t* const address = internal::futex_address_from_word(word);

    (void)internal::futex_wake_private(address, INT_MAX);
}

#else

void wait_while_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept
{
    (void)word;
    (void)value;
}

void wake_one_waiter(const std::atomic<std::uint32_t>& word) noexcept
{
    (void)word;
}

void wake_all_waiters(const std::atomic<std::uint32_t>& word) noexcept
{
    (void)word;
}

#if defined(MV_PLATFORM_APPLE)

#error "platform::threading atomic wait/wake is not implemented for macOS in phase 1."

#elif defined(MV_PLATFORM_ANDROID_ONLY)

#error "platform::threading atomic wait/wake is not implemented for Android in phase 1."

#else

#error "platform::threading atomic wait/wake is not implemented for this platform."

#endif

#endif

}   //  namespace platform::threading
