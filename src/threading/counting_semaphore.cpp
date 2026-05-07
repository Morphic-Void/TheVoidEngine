
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   counting_semaphore.cpp
//  Author: Ritchie Brannan
//  Date:   5 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Process-local counting semaphore implementation.

#include "threading/counting_semaphore.hpp"
#include "platform/threading/wait_word.hpp"

namespace threading
{

constexpr std::uint32_t k_semaphore_max_count = ~std::uint32_t{ 0 };

void CCountingSemaphore::initialise(const std::uint32_t initial_count) noexcept
{
    m_count.store(initial_count, std::memory_order_relaxed);
}

void CCountingSemaphore::destroy() noexcept
{
    m_count.store(0u, std::memory_order_relaxed);
}

std::uint32_t CCountingSemaphore::query_count() const noexcept
{
    return m_count.load(std::memory_order_relaxed);
}

void CCountingSemaphore::acquire() noexcept
{
    std::uint32_t seen = m_count.load(std::memory_order_acquire);

    for (;;)
    {
        while (seen == 0u)
        {
            platform::threading::wait_while_equal(m_count, 0u);

            seen = m_count.load(std::memory_order_acquire);
        }

        const std::uint32_t wanted = seen - 1u;

        if (m_count.compare_exchange_weak(seen, wanted,
            std::memory_order_acquire, std::memory_order_relaxed))
        {
            break;
        }

        //  On failure, seen has been updated with the current m_count.
    }
}

bool CCountingSemaphore::try_acquire() noexcept
{
    std::uint32_t seen = m_count.load(std::memory_order_acquire);

    while (seen != 0u)
    {
        const std::uint32_t wanted = seen - 1u;

        if (m_count.compare_exchange_weak(seen, wanted,
            std::memory_order_acquire, std::memory_order_relaxed))
        {
            return true;
        }

        //  On failure, seen has been updated with the current m_count.
    }

    return false;
}

bool CCountingSemaphore::release(const std::uint32_t release_count) noexcept
{
    if (release_count == 0u)
    {
        return true;
    }

    std::uint32_t seen = m_count.load(std::memory_order_relaxed);

    for (;;)
    {
        if (release_count > (k_semaphore_max_count - seen))
        {
            return false;
        }

        const std::uint32_t wanted = seen + release_count;

        if (m_count.compare_exchange_weak(seen, wanted,
            std::memory_order_release, std::memory_order_relaxed))
        {
            if (release_count == 1u)
            {
                platform::threading::wake_one_waiter(m_count);
            }
            else
            {
                platform::threading::wake_all_waiters(m_count);
            }

            return true;
        }

        //  On failure, seen has been updated with the current m_count.
    }
}

}   //  namespace threading
