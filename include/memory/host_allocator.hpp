
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   host_allocator.hpp
//  Author: Ritchie Brannan
//  Date:   8 May 26
//
//  Host allocator access gate.
//
//  Provides controlled access to the executable-owned process allocator.
//
//  Only the authorised host system identity may fetch this allocator directly.
//  Modules must obtain allocator access through their host-provided services.
//
//  Design constraints
//  ------------------
//  - Requires C++17 or later.
//  - No exceptions.

#pragma once

#ifndef HOST_ALLOCATOR_HPP_INCLUDED
#define HOST_ALLOCATOR_HPP_INCLUDED

#include <cstddef>      //  std::size_t

#include "allocation_context.hpp"

namespace memory
{

//  Fetch the executable-owned host allocator.
//
//  Succeeds only for the authorised host system identity.
//  Modules must not call this directly.
IAllocator* get_the_host_allocator(const std::size_t system_id) noexcept;

}   //  namespace memory

#endif  //  #ifndef HOST_ALLOCATOR_HPP_INCLUDED
