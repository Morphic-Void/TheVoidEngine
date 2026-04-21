
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TOwningBundle.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport bundle for owned T.

#pragma once

#ifndef TOWNING_BUNDLE_HPP_INCLUDED
#define TOWNING_BUNDLE_HPP_INCLUDED

#include "threading/transports/core/TOwningTransport.hpp"
#include "threading/transports/endpoints/TOwningEndpoints.hpp"

namespace threading::transports
{

//==============================================================================
//  TOwningBundle<T>
//  Single Producer, Single Consumer (SPSC) transport bundle
//==============================================================================

template<typename T>
struct TOwningBundle
{
    TOwning<T> transport;
    TOwningProducerEndpoint<T> producer;
    TOwningConsumerEndpoint<T> consumer;

    TOwningBundle() noexcept : transport(), producer(transport), consumer(transport) {}
};

}   //  namespace threading::transports

#endif  //  #ifndef TOWNING_BUNDLE_HPP_INCLUDED

