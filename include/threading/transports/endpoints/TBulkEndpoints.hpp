
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TBulkEndpoints.hpp
//  Author: Ritchie Brannan
//  Date:   25 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC latest-state transport endpoints for bulk payloads.

#pragma once

#ifndef TBULK_ENDPOINTS_HPP_INCLUDED
#define TBULK_ENDPOINTS_HPP_INCLUDED

#include "threading/transports/TBulkTransport.hpp"
#include "threading/transports/interfaces/TBulkInterfaces.hpp"

namespace threading::transports
{

//==============================================================================
//  TBulkProducerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) producer endpoint
//==============================================================================

template<typename T>
class TBulkProducerEndpoint : public TBulkProducerInterface<T>
{
public:
    explicit TBulkProducerEndpoint(TBulk<T>& bulk) noexcept : m_bulk{ bulk } {}
    ~TBulkProducerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual T* producer_data() noexcept override final;
    virtual bool publish() noexcept override final;

private:
    TBulk<T>& m_bulk;
};

//==============================================================================
//  TBulkConsumerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) consumer endpoint
//==============================================================================

template<typename T>
class TBulkConsumerEndpoint : public TBulkConsumerInterface<T>
{
public:
    explicit TBulkConsumerEndpoint(TBulk<T>& bulk) noexcept : m_bulk{ bulk } {}
    ~TBulkConsumerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual T* consumer_data() noexcept override final;
    virtual bool discard_and_acquire() noexcept override final;

private:
    TBulk<T>& m_bulk;
};

//==============================================================================
//  TBulkProducerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline T* TBulkProducerEndpoint<T>::producer_data() noexcept
{
    return m_bulk.producer_data();
}

template<typename T>
inline bool TBulkProducerEndpoint<T>::publish() noexcept
{
    return m_bulk.publish();
}

//==============================================================================
//  TBulkConsumerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline T* TBulkConsumerEndpoint<T>::consumer_data() noexcept
{
    return m_bulk.consumer_data();
}

template<typename T>
inline bool TBulkConsumerEndpoint<T>::discard_and_acquire() noexcept
{
    return m_bulk.discard_and_acquire();
}

}   //  namespace threading::transports

#endif  //  #ifndef TBULK_ENDPOINTS_HPP_INCLUDED
