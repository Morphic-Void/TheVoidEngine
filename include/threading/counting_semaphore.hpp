//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   counting_semaphore.hpp
//  Author: Ritchie Brannan
//  Date:   5 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Process-local counting semaphore.
//
//  Provides a counted permit primitive over atomics and platform waits.
//  Permits are remembered until consumed.
//  acquire() consumes one permit, blocking while none are available.
//  try_acquire() consumes one permit only if one is currently available.
//  release() adds permits and wakes waiters.
//
//  Associated queues, pools, work lists, and shutdown state remain the
//  responsibility of the caller.

#pragma once

#ifndef COUNTING_SEMAPHORE_HPP_INCLUDED
#define COUNTING_SEMAPHORE_HPP_INCLUDED

#include <atomic>
#include <cstdint>

namespace threading
{

class CCountingSemaphore
{
public:

    //  Default and deleted lifetime
    CCountingSemaphore() noexcept = default;
    CCountingSemaphore(const CCountingSemaphore&) = delete;
    CCountingSemaphore& operator=(const CCountingSemaphore&) = delete;
    CCountingSemaphore(CCountingSemaphore&&) = delete;
    CCountingSemaphore& operator=(CCountingSemaphore&&) = delete;
    ~CCountingSemaphore() noexcept = default;

    //  Initial and final states
    void initialise(const std::uint32_t initial_count = 0u) noexcept;
    void destroy() noexcept;

    //  Weak status
    std::uint32_t query_count() const noexcept;

    //  Consumer accessors
    void acquire() noexcept;
    bool try_acquire() noexcept;

    //  Producer permit issuance
    bool release(const std::uint32_t release_count = 1u) noexcept;

private:
    std::atomic<std::uint32_t> m_count{ 0u };
};

}   //  namespace threading

#endif  //  #ifndef COUNTING_SEMAPHORE_HPP_INCLUDED
