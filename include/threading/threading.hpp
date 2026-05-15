
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   threading.hpp
//  Author: Ritchie Brannan
//  Date:   15 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Convenience include for the platform agnostic threading primitives.
//
//  Include individual headers when a narrower dependency is preferred.

#pragma once

#ifndef THREADING_HPP_INCLUDED
#define THREADING_HPP_INCLUDED

#include "threading/CParkingGate.hpp"
#include "threading/CPodThreadMsg.hpp"
#include "threading/CThreadControlState.hpp"
#include "threading/CWaitPredicate.hpp"
#include "threading/CCountingSemaphore.hpp"
#include "threading/transports/bundles/TBulkBundle.hpp"
#include "threading/transports/bundles/TOwningBundle.hpp"
#include "threading/transports/bundles/TQueueBundle.hpp"
#include "threading/transports/bundles/TRingBundle.hpp"

#endif  //  #ifndef THREADING_HPP_INCLUDED
