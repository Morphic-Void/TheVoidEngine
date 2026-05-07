//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   wait_word.hpp
//  Author: Ritchie Brannan
//  Date:   7 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Low-level atomic wait/wake primitives for 32-bit wait words.
//
//  These functions expose platform wait-on-address style blocking over an
//  externally owned atomic word. They are intended for building higher-level
//  predicate waits, counters, semaphores, and other atomic state-machine loops.
//
//  Wakeups are not remembered. The wait operation may return spuriously or
//  after the observed word has changed. Callers that need a stable condition
//  must re-check their controlling predicate after each return.
//
//  These functions do not modify the atomic word and do not provide
//  acquire/release synchronization. Required ordering must be provided by the
//  caller's atomic operations.
//
//  The wait word must remain alive for the duration of any wait.

#pragma once

#ifndef WAIT_WORD_HPP_INCLUDED
#define WAIT_WORD_HPP_INCLUDED

#include <atomic>       //  std::atomic
#include <cstdint>      //  std::uint32_t

namespace platform::threading
{

//==============================================================================
//  Atomic wait word primitives
//==============================================================================

//  Blocks the calling thread while word equals expected.
void wait_while_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept;

//  Wakes one current waiter blocked on word, if one exists.
void wake_one_waiter(const std::atomic<std::uint32_t>& word) noexcept;

//  Wakes all current waiters blocked on word, if any exist.
void wake_all_waiters(const std::atomic<std::uint32_t>& word) noexcept;

}   //  namespace platform::threading

#endif  //  #ifndef WAIT_WORD_HPP_INCLUDED
