
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   wait_predicates.hpp
//  Author: Ritchie Brannan
//  Date:   5 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Platform-agnostic predicate wait helpers over platform wait primitives.
//
//  Provides completed equality/inequality predicate loops.
//  The raw wait primitive remains platform::threading::wait_while_equal().

#pragma once

#ifndef WAIT_PREDICATES_HPP_INCLUDED
#define WAIT_PREDICATES_HPP_INCLUDED

#include <atomic>       //  std::atomic
#include <cstdint>      //  std::uint32_t

#include "platform/threading/wait_word.hpp"

namespace threading
{

//  Wait until word is seen to equal value.
//
//  This waits for a specific visible value, not just for progress.
//  word must remain alive and stable for the duration of the wait.
inline void wait_until_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept
{
    for (;;)
    {
        const std::uint32_t seen = word.load(std::memory_order_acquire);
    
        if (seen == value)
        {
            break;
        }
    
        platform::threading::wait_while_equal(word, seen);
    }
}

//  Wait until word is seen to differ from value.
//  Returns the seen non-equal value.
//
//  word must remain alive and stable for the duration of the wait.
inline std::uint32_t wait_until_not_equal(const std::atomic<std::uint32_t>& word, const std::uint32_t value) noexcept
{
    for (;;)
    {
        const std::uint32_t seen = word.load(std::memory_order_acquire);
    
        if (seen != value)
        {
            return seen;
        }
    
        platform::threading::wait_while_equal(word, value);
    }
}

}   //  namespace threading

#endif  //  #ifndef WAIT_PREDICATES_HPP_INCLUDED
