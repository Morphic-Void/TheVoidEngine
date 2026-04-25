
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TBulkInterfaces.hpp
//  Author: Ritchie Brannan
//  Date:   25 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC latest-state transport interfaces for bulk payloads.

#pragma once

#ifndef TBULK_INTERFACES_HPP_INCLUDED
#define TBULK_INTERFACES_HPP_INCLUDED

namespace threading::transports
{



//  Consumer operations
//  - only safe to call from the consumer thread or while quiescent
//  - consumer_data() returns the currently acquired consumer payload,
//    or nullptr if none is active; it does not acquire new data
//  - discard_and_acquire() always discards any active consumer data and
//    returns true if new data was acquired.


//==============================================================================
//  TBulkProducerInterface<T>
//  Single Producer, Single Consumer (SPSC) producer interface
//==============================================================================

template<typename T>
class TBulkProducerInterface
{
public:
    [[nodiscard]] virtual T* producer_data() noexcept;
    virtual bool publish() noexcept;

protected:
    TBulkProducerInterface() noexcept = default;
    ~TBulkProducerInterface() noexcept = default;
};

//==============================================================================
//  TBulkConsumerInterface<T>
//  Single Producer, Single Consumer (SPSC) consumer interface
//==============================================================================

template<typename T>
class TBulkConsumerInterface
{
public:
    [[nodiscard]] virtual T* consumer_data() noexcept = 0;
    virtual bool discard_and_acquire() noexcept = 0;

protected:
    TBulkConsumerInterface() noexcept = default;
    ~TBulkConsumerInterface() noexcept = default;
};

}   //  namespace threading::transports

#endif  //  #ifndef TBULK_INTERFACES_HPP_INCLUDED

