
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TOwningInterfaces.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport interfaces for owned T.

#pragma once

#ifndef TOWNING_INTERFACES_HPP_INCLUDED
#define TOWNING_INTERFACES_HPP_INCLUDED

namespace threading::transports
{

//==============================================================================
//  TOwningProducerInterface<T>
//  Single Producer, Single Consumer (SPSC) producer interface
//==============================================================================

template<typename T>
class TOwningProducerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool post(T&& src) noexcept = 0;
    [[nodiscard]] virtual std::uint32_t writable_count() const noexcept = 0;

protected:
    TOwningProducerInterface() noexcept = default;
    ~TOwningProducerInterface() noexcept = default;
};

//==============================================================================
//  TOwningConsumerInterface<T>
//  Single Producer, Single Consumer (SPSC) consumer interface
//==============================================================================

template<typename T>
class TOwningConsumerInterface
{
public:
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    [[nodiscard]] virtual bool is_ready() const noexcept = 0;
    [[nodiscard]] virtual bool read(T& dst) noexcept = 0;
    [[nodiscard]] virtual std::uint32_t readable_count() const noexcept = 0;

protected:
    TOwningConsumerInterface() noexcept = default;
    ~TOwningConsumerInterface() noexcept = default;
};

}   //  namespace threading::transports

#endif  //  #ifndef TOWNING_INTERFACES_HPP_INCLUDED

