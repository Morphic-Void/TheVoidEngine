
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TRingBundle.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport bundle for trivially copyable T.

#pragma once

#ifndef TRING_BUNDLE_HPP_INCLUDED
#define TRING_BUNDLE_HPP_INCLUDED

#include "threading/transports/core/TRingTransport.hpp"
#include "threading/transports/endpoints/TRingEndpoints.hpp"

namespace threading::transports
{

//==============================================================================
//  TRingBundle<T>
//  Single Producer, Single Consumer (SPSC) transport bundle
//==============================================================================

template<typename T>
struct TRingBundle
{
    TRing<T> transport;
    TRingProducerEndpoint<T> producer;
    TRingConsumerEndpoint<T> consumer;

    TRingBundle() noexcept : transport(), producer(transport), consumer(transport) {}
};

}   //  namespace threading::transports

#endif  //  #ifndef TRING_BUNDLE_HPP_INCLUDED

