
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TRingInterfaces.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport interfaces for trivially copyable T.
//
//  IMPORTANT SEMANTIC NOTE
//  -----------------------
//  post() and read() are all-or-nothing.

#pragma once

#ifndef TRING_INTERFACES_HPP_INCLUDED
#define TRING_INTERFACES_HPP_INCLUDED

#include "containers/TPodVector.hpp"

namespace threading::transports
{

//==============================================================================
//  TRingProducerInterface<T>
//  Single Producer, Single Consumer (SPSC) producer interface
//==============================================================================

template<typename T>
class TRingProducerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool post(const T& src) noexcept = 0;
    [[nodiscard]] virtual bool post(const T* const src, const std::uint32_t count = 1u) noexcept = 0;
    [[nodiscard]] virtual bool post(const TPodConstView<T>& src) noexcept = 0;
    [[nodiscard]] virtual std::uint32_t writable_count() const noexcept = 0;

protected:
    TRingProducerInterface() noexcept = default;
    ~TRingProducerInterface() noexcept = default;
};

//==============================================================================
//  TRingConsumerInterface<T>
//  Single Producer, Single Consumer (SPSC) consumer interface
//==============================================================================

template<typename T>
class TRingConsumerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool read(T& dst) noexcept = 0;
    [[nodiscard]] virtual bool read(T* const dst, const std::uint32_t count = 1u) noexcept = 0;
    [[nodiscard]] virtual bool read(const TPodView<T>& dst) noexcept = 0;
    [[nodiscard]] virtual std::uint32_t readable_count() const noexcept = 0;

protected:
    TRingConsumerInterface() noexcept = default;
    ~TRingConsumerInterface() noexcept = default;
};

}   //  namespace threading::transports

#endif  //  #ifndef TRING_INTERFACES_HPP_INCLUDED

