
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TOwningTransport.hpp
//  Author: Ritchie Brannan
//  Date:   21 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport for owned T.
//
//  Does not grow, discard, overwrite unread data, or provide
//  blocking semantics.
//
//  IMPORTANT SEMANTIC NOTE
//  -----------------------
//  Slots are live T objects after initialise() and remain live until deallocate().
//
//  Occupancy controls readability and writability, not object lifetime.
//
//  writable_count() and readable_count() are snapshot observations,
//  not reservation mechanisms.
//
//  See docs/threading/transports/TOwningTransport.md for the full documentation.

#pragma once

#ifndef TOWNING_TRANSPORT_HPP_INCLUDED
#define TOWNING_TRANSPORT_HPP_INCLUDED

#include <algorithm>    //  std::min, std::max
#include <atomic>       //  std::atomic
#include <cstddef>      //  std::size_t
#include <cstdint>      //  std::uint32_t
#include <cstring>      //  std::memcpy
#include <type_traits>  //  std::is_const_v, std::is_trivially_copyable_v

#include "memory/memory_primitives.hpp"
#include "bit_utils/bit_ops.hpp"

namespace threading::transports
{

//==============================================================================
//  TOwning<T>
//  Single Producer, Single Consumer (SPSC) transport
//==============================================================================

template<typename T>
class TOwning
{
private:
    static_assert(!std::is_copy_constructible_v<T>, "TOwning<T> requires non-copy constructible T.");
    static_assert(!std::is_copy_assignable_v<T>, "TOwning<T> requires non-copy assignable T.");
    static_assert(std::is_nothrow_default_constructible_v<T>, "TOwning<T> requires nothrow default constructible T.");
    static_assert(std::is_nothrow_destructible_v<T>, "TOwning<T> requires nothrow destructible T.");
    static_assert(std::is_nothrow_move_constructible_v<T>, "TOwning<T> requires nothrow move constructible T.");
    static_assert(std::is_nothrow_move_assignable_v<T>, "TOwning<T> requires nothrow move assignable T.");

public:
    static constexpr std::uint32_t k_max_capacity = 0x00100000u;    //  approximately 1 million elements
    static constexpr std::uint32_t k_min_capacity = 32u;

public:
    TOwning() noexcept = default;
    TOwning(const TOwning&) = delete;
    TOwning& operator=(const TOwning&) = delete;
    TOwning(TOwning&&) noexcept = delete;
    TOwning& operator=(TOwning&&) noexcept = delete;
    ~TOwning() noexcept { deallocate(); }

    //  Shared status
    [[nodiscard]] bool is_valid() const noexcept;
    [[nodiscard]] bool is_ready() const noexcept;

    //  Producer status and operations
    //  - only safe to call from the producer thread or while quiescent
    [[nodiscard]] bool posting_is_valid() const noexcept;
    [[nodiscard]] bool post(T&& src) noexcept;
    [[nodiscard]] std::uint32_t writable_count() const noexcept;

    //  Consumer status and operations
    //  - only safe to call from the consumer thread or while quiescent
    [[nodiscard]] bool reading_is_valid() const noexcept;
    [[nodiscard]] bool read(T& dst) noexcept;
    [[nodiscard]] std::uint32_t readable_count() const noexcept;

    //  Setup and teardown
    //  initialise() requires a deallocated / not-ready instance.
    //  deallocate() releases owned storage and must not race active role use.
    [[nodiscard]] bool initialise(const std::uint32_t capacity) noexcept;
    void deallocate() noexcept;

private:
    memory::TMemoryToken<T> m_ring;
    std::uint32_t m_capacity = 0u;
    std::uint32_t m_read_index = 0u;
    std::uint32_t m_write_index = 0u;
    std::atomic<std::uint32_t> m_occupied_count{ 0u };
};

//==============================================================================
//  TOwning<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TOwning<T>::is_valid() const noexcept
{
    if (m_capacity == 0u)
    {   //  uninitialised, safe to check both the read and the write indices
        if ((m_ring.data() != nullptr) || ((m_read_index | m_write_index) != 0u))
        {
            return false;
        }
    }
    else if (m_ring.data() == nullptr)
    {
        return false;
    }
    return m_occupied_count.load(std::memory_order_acquire) <= m_capacity;
}

template<typename T>
inline bool TOwning<T>::is_ready() const noexcept
{
    return (m_capacity != 0u) && (m_ring.data() != nullptr);
}

template<typename T>
inline bool TOwning<T>::posting_is_valid() const noexcept
{
    return is_valid() ? ((m_capacity != 0u) ? (m_write_index < m_capacity) : (m_write_index == 0u)) : false;
}

template<typename T>
inline bool TOwning<T>::post(T&& src) noexcept
{
    if (!is_ready() || (writable_count() == 0u))
    {
        return false;
    }
    m_ring.data()[m_write_index] = std::move(src);
    m_write_index = (m_write_index + 1u) & (m_capacity - 1u);
    m_occupied_count.fetch_add(1u, std::memory_order_release);
    return true;
}

template<typename T>
inline std::uint32_t TOwning<T>::writable_count() const noexcept
{
    const std::uint32_t count = m_occupied_count.load(std::memory_order_acquire);
    return (count <= m_capacity) ? (m_capacity - count) : 0u;
}

template<typename T>
inline bool TOwning<T>::reading_is_valid() const noexcept
{
    return is_valid() ? ((m_capacity != 0u) ? (m_read_index < m_capacity) : (m_read_index == 0u)) : false;
}

template<typename T>
inline bool TOwning<T>::read(T& dst) noexcept
{
    if (!is_ready() || (readable_count() == 0u))
    {
        return false;
    }
    dst = std::move(m_ring.data()[m_read_index]);
    m_read_index = (m_read_index + 1u) & (m_capacity - 1u);
    m_occupied_count.fetch_sub(1u, std::memory_order_release);
    return true;
}

template<typename T>
inline std::uint32_t TOwning<T>::readable_count() const noexcept
{
    const std::uint32_t count = m_occupied_count.load(std::memory_order_acquire);
    return (count <= m_capacity) ? count : 0u;
}

template<typename T>
inline bool TOwning<T>::initialise(const std::uint32_t capacity) noexcept
{
    if (capacity > k_max_capacity)
    {   //  requested capacity not supported
        return false;
    }
    if (m_capacity != 0u)
    {   //  re-initialisation is not allowed without deallocation
        return false;
    }
    const std::uint32_t conditioned_capacity = std::max(std::min(bit_ops::round_up_to_pow2(capacity), k_max_capacity), k_min_capacity);
    if (!m_ring.allocate(conditioned_capacity))
    {   //  allocation failed
        return false;
    }
    m_capacity = conditioned_capacity;
    m_read_index = 0u;
    m_write_index = 0u;
    m_occupied_count.store(0u, std::memory_order_release);
    for (std::uint32_t index = 0u; index < m_capacity; ++index)
    {
        ::new (static_cast<void*>(&m_ring.data()[index])) T();
    }
    return true;
}

template<typename T>
inline void TOwning<T>::deallocate() noexcept
{
    if (m_ring.data() != nullptr)
    {
        for (std::uint32_t index = 0u; index < m_capacity; ++index)
        {
            m_ring.data()[index].~T();
        }
    }
    m_ring.deallocate();
    m_capacity = 0u;
    m_read_index = 0u;
    m_write_index = 0u;
    m_occupied_count.store(0u, std::memory_order_release);
}

}   //  namespace threading::transports

#endif  //  #ifndef TOWNING_TRANSPORT_HPP_INCLUDED

