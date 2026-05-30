
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
// 
//  File:   allocation_context.cpp
//  Author: Ritchie Brannan
//  Date:   29 May 26
//
//  Thread-local allocation context routing.
//
//  Implements the thread-local current-context pointer and the forwarding
//  functions declared in allocation_context.hpp.

#include <cstddef>      //  std::size_t
#include <cstdint>      //  std::uint32_t

#include "memory/allocation_context.hpp"

namespace memory::context
{

//==============================================================================
//  Thread and module local context
//==============================================================================

thread_local CAllocationContext* g_allocator_context{ nullptr };

//==============================================================================
//  Context installation
//==============================================================================

bool install(CAllocationContext& context) noexcept
{
    if (g_allocator_context == nullptr)
    {
        g_allocator_context = &context;
        return true;
    }
    return false;
}

bool uninstall(const CAllocationContext& context) noexcept
{
    if ((g_allocator_context == &context) && !context.is_in_use())
    {
        g_allocator_context = nullptr;
        return true;
    }
    return false;
}

//==============================================================================
//  Contextual allocation
//==============================================================================

void* byte_allocate(const std::size_t bytes, const std::size_t align) noexcept
{
    if (g_allocator_context != nullptr)
    {   //  zero size allocations are rejected
        CAllocationContext& context = *g_allocator_context;
        return context.byte_allocate(bytes, align);
    }
    return nullptr;
}

bool byte_deallocate(void* const ptr, const std::size_t align) noexcept
{
    if (g_allocator_context != nullptr)
    {   //  note: the ptr null check is not strictly required
        CAllocationContext& context = *g_allocator_context;
        return context.byte_deallocate(ptr, align);
    }
    return false;
}

}   //  namespace memory::context
