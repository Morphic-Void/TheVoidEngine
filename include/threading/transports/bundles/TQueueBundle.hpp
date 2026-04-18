
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TQueueBundle.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC sequential transport bundle for trivially copyable T.

#pragma once

#ifndef TQUEUE_BUNDLE_HPP_INCLUDED
#define TQUEUE_BUNDLE_HPP_INCLUDED

#include "threading/transports/core/TQueueTransport.hpp"
#include "threading/transports/endpoints/TQueueEndpoints.hpp"

namespace threading::transports
{

//==============================================================================
//  TQueueBundle<T>
//  Single Producer, Single Consumer (SPSC) transport bundle
//==============================================================================

template<typename T>
struct TQueueBundle
{
    TQueue<T> transport;
    TQueueProducerEndpoint<T> producer;
    TQueueConsumerEndpoint<T> consumer;

    TQueueBundle() noexcept : transport(), producer(transport), consumer(transport) {}
};

}   //  namespace threading::transports

#endif  //  #ifndef TQUEUE_BUNDLE_HPP_INCLUDED
