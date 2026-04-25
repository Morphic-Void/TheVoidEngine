
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TBulkBundle.hpp
//  Author: Ritchie Brannan
//  Date:   25 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC latest-state transport bundle for bulk payloads.

#pragma once

#ifndef TBULK_BUNDLE_HPP_INCLUDED
#define TBULK_BUNDLE_HPP_INCLUDED

#include "threading/transports/core/TBulkTransport.hpp"
#include "threading/transports/endpoints/TBulkEndpoints.hpp"

namespace threading::transports
{

//==============================================================================
//  TBulkBundle<T>
//  Single Producer, Single Consumer (SPSC) transport bundle
//==============================================================================

template<typename T>
struct TBulkBundle
{
    TBulk<T> transport;
    TBulkProducerEndpoint<T> producer;
    TBulkConsumerEndpoint<T> consumer;

    TBulkBundle() noexcept : transport(), producer(transport), consumer(transport) {}
};

}   //  namespace threading::transports

#endif  //  #ifndef TBULK_BUNDLE_HPP_INCLUDED

