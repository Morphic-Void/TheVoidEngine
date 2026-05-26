
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TQueueInterfaces.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC sequential transport interfaces for trivially copyable T.
//
//  IMPORTANT SEMANTIC NOTE
//  -----------------------
//  post() and read() are all-or-nothing.

#pragma once

#ifndef TQUEUE_INTERFACES_HPP_INCLUDED
#define TQUEUE_INTERFACES_HPP_INCLUDED

#include "containers/TPodVector.hpp"

namespace threading::transports
{

//==============================================================================
//  TQueueProducerInterface<T>
//  Single Producer, Single Consumer (SPSC) producer interface
//==============================================================================

template<typename T>
class TQueueProducerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool is_poisoned() const noexcept = 0;
    [[nodiscard]] virtual bool post(const T& src) noexcept = 0;
    [[nodiscard]] virtual bool post(const T* const src, const std::uint32_t count = 1u) noexcept = 0;
    [[nodiscard]] virtual bool post(const TPodConstView<T>& src) noexcept = 0;
    [[nodiscard]] virtual bool post_would_reallocate(const std::uint32_t count) const noexcept = 0;

protected:
    TQueueProducerInterface() noexcept = default;
    ~TQueueProducerInterface() noexcept = default;
};

//==============================================================================
//  TQueueConsumerInterface<T>
//  Single Producer, Single Consumer (SPSC) consumer interface
//==============================================================================

template<typename T>
class TQueueConsumerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool read(T& dst) noexcept = 0;
    [[nodiscard]] virtual bool read(T* const dst, const std::uint32_t count = 1u) noexcept = 0;
    [[nodiscard]] virtual bool read(const TPodView<T>& dst) noexcept = 0;
    [[nodiscard]] virtual std::uint32_t current_readable_count() const noexcept = 0;
    [[nodiscard]] virtual std::uint32_t refresh_readable_count() noexcept = 0;

protected:
    TQueueConsumerInterface() noexcept = default;
    ~TQueueConsumerInterface() noexcept = default;
};

}   //  namespace threading::transports

#endif  //  #ifndef TQUEUE_INTERFACES_HPP_INCLUDED
