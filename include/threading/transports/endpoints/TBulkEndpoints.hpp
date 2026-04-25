
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

#include "threading/transports/core/TBulkTransport.hpp"
#include "threading/transports/interfaces/TBulkInterfaces.hpp"
#include "debug/debug.hpp"

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
    explicit TBulkProducerEndpoint(TBulk<T>& bulk) noexcept : m_bulk{ &bulk } {}
    ~TBulkProducerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual T* producer_data() noexcept override final;
    virtual bool publish() noexcept override final;

private:
    TBulk<T>* m_bulk = nullptr;
};

//==============================================================================
//  TBulkConsumerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) consumer endpoint
//==============================================================================

template<typename T>
class TBulkConsumerEndpoint : public TBulkConsumerInterface<T>
{
public:
    explicit TBulkConsumerEndpoint(TBulk<T>& bulk) noexcept : m_bulk{ &bulk } {}
    ~TBulkConsumerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual T* consumer_data() noexcept override final;
    virtual bool discard_and_acquire() noexcept override final;

private:
    TBulk<T>* m_bulk = nullptr;
};

//==============================================================================
//  TBulkProducerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline T* TBulkProducerEndpoint<T>::producer_data() noexcept
{
    MV_HARD_ASSERT(m_bulk != nullptr);
    return (m_bulk != nullptr) ? m_bulk->producer_data() : nullptr;
}

template<typename T>
inline bool TBulkProducerEndpoint<T>::publish() noexcept
{
    MV_HARD_ASSERT(m_bulk != nullptr);
    return (m_bulk != nullptr) ? m_bulk->publish() : false;
}

//==============================================================================
//  TBulkConsumerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline T* TBulkConsumerEndpoint<T>::consumer_data() noexcept
{
    MV_HARD_ASSERT(m_bulk != nullptr);
    return (m_bulk != nullptr) ? m_bulk->consumer_data() : nullptr;
}

template<typename T>
inline bool TBulkConsumerEndpoint<T>::discard_and_acquire() noexcept
{
    MV_HARD_ASSERT(m_bulk != nullptr);
    return (m_bulk != nullptr) ? m_bulk->discard_and_acquire() : false;
}

}   //  namespace threading::transports

#endif  //  #ifndef TBULK_ENDPOINTS_HPP_INCLUDED
