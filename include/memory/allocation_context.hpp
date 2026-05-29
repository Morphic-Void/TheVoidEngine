
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
// 
//  File:   allocation_context.hpp
//  Author: Ritchie Brannan
//  Date:   29 May 26
//
//  Allocation context interface and state.
//
//  Purpose
//  -------
//  Defines the per-thread allocation context used to route raw byte allocation
//  requests through an externally owned allocator interface.
//
//  This header contains:
//
//      - IAllocator, the non-owning raw byte allocator interface
//      - CAllocationContext, the allocator binding and local accounting state
//      - memory::context declarations for installing a thread-local current
//        context and routing allocation/deallocation through it
//
//  Scope
//  -----
//  This file defines routing state only. It does not own heap storage, define
//  allocation policy, construct objects, normalize alignment, or provide typed
//  container semantics.
//
//  State model
//  -----------
//  A CAllocationContext contains a caller-defined system id, a non-owning
//  allocator pointer, and local allocation counters.
//
//  The allocator pointer may only be changed while the context has no recorded
//  live allocations. is_usable() reports whether the context has an allocator
//  currently available for routing.
//
//  Successful non-zero byte allocations increase the live, peak, and lifetime
//  counters. Successful deallocations decrease the live counter.
//
//  Ownership and lifetime
//  ----------------------
//  CAllocationContext does not own the allocator it references. The allocator
//  must remain valid for every allocation and deallocation routed through the
//  context.
//
//  Threading model
//  ---------------
//  The installed current context is thread-local. A context is expected to be
//  provisioned before the owning thread installs it, mutated only by that
//  thread while installed, and inspected externally only after the owning thread
//  has stopped using it.
//
//  The counters are local accounting fields, not atomic telemetry. They do not
//  support concurrent host observation while the owning thread is active.

#pragma once

#ifndef ALLOCATION_CONTEXT_HPP_INCLUDED
#define ALLOCATION_CONTEXT_HPP_INCLUDED

#include <algorithm>    //  std::min, std::max
#include <cstddef>      //  std::size_t
#include <cstdint>      //  std::uint32_t

namespace memory
{

//==============================================================================
//  Forward declarations
//==============================================================================

class IAllocator;
class CAllocationContext;

//==============================================================================
//  Installation and allocation
//==============================================================================

namespace context
{

bool install(CAllocationContext& context) noexcept;
bool uninstall(const CAllocationContext& context) noexcept;
void* byte_allocate(const std::size_t bytes, const std::size_t align) noexcept;
void byte_deallocate(void* const ptr, const std::size_t align) noexcept;

}   //  namespace context

//==============================================================================
//  IAllocator
//  - abstract allocator interface
//==============================================================================

class IAllocator
{
public:
    virtual void* byte_allocate(const std::size_t bytes, const std::size_t align) noexcept = 0;
    virtual void byte_deallocate(void* const ptr, const std::size_t align) noexcept = 0;
};

//==============================================================================
//  CAllocationContext
//  - per-thread, per-module allocator context
//==============================================================================

class CAllocationContext
{
public:

    //  Default and deleted lifetime
    CAllocationContext() noexcept = default;
    CAllocationContext(const CAllocationContext&) noexcept = delete;
    CAllocationContext& operator=(const CAllocationContext&) noexcept = delete;
    CAllocationContext(CAllocationContext&&) noexcept = delete;
    CAllocationContext& operator=(CAllocationContext&&) noexcept = delete;
    ~CAllocationContext() noexcept = default;

    //  Configuration
    std::size_t set_system_id(const std::size_t system_id = 0u) noexcept;
    std::size_t get_system_id() const noexcept;
    bool        set_allocator(IAllocator* const allocator = nullptr) noexcept;
    IAllocator* get_allocator() const noexcept;

    //  Status
    bool is_usable() const noexcept;
    bool is_in_use() const noexcept;

    //  Allocation and deallocation
    void* byte_allocate(const std::size_t bytes, const std::size_t align) noexcept;
    void  byte_deallocate(void* const ptr, const std::size_t align) noexcept;

    //  Statistics queries
    std::uint32_t get_live_allocation_count() const noexcept;
    std::uint32_t get_peak_allocation_count() const noexcept;
    std::uint64_t get_lifetime_allocation_count() const noexcept;

private:
    std::size_t     m_system_id{ 0u };
    IAllocator*     m_allocator{ nullptr };
    std::uint32_t   m_live_allocation_count{ 0u };
    std::uint32_t   m_peak_allocation_count{ 0u };
    std::uint64_t   m_lifetime_allocation_count{ 0ull };
};

//==============================================================================
//  CAllocationContext out of class function bodies
//==============================================================================

inline std::size_t CAllocationContext::set_system_id(const std::size_t system_id) noexcept
{
    std::size_t previous_system_id = m_system_id;
    m_system_id = system_id;
    return previous_system_id;
}

inline std::size_t CAllocationContext::get_system_id() const noexcept
{
    return m_system_id;
}

inline bool CAllocationContext::set_allocator(IAllocator* const allocator) noexcept
{
    if (m_live_allocation_count == 0u)
    {
        m_allocator = allocator;
        return true;
    }
    return false;
}

inline IAllocator* CAllocationContext::get_allocator() const noexcept
{
    return m_allocator;
}

inline bool CAllocationContext::is_usable() const noexcept
{
    return m_allocator != nullptr;
}

inline bool CAllocationContext::is_in_use() const noexcept
{
    return m_live_allocation_count != 0u;
}

inline void* CAllocationContext::byte_allocate(const std::size_t bytes, const std::size_t align) noexcept
{
    if (m_allocator != nullptr)
    {
        if (bytes != 0u)
        {   //  note: zero size allocations are rejected
            void* ptr = m_allocator->byte_allocate(bytes, align);
            if (ptr != nullptr)
            {
                m_live_allocation_count += 1u;
                m_peak_allocation_count = std::max(m_live_allocation_count, m_peak_allocation_count);
                m_lifetime_allocation_count += 1u;
                return ptr;
            }
        }
    }
    return nullptr;
}

inline void CAllocationContext::byte_deallocate(void* const ptr, const std::size_t align) noexcept
{
    if (m_allocator != nullptr)
    {
        if (ptr != nullptr)
        {   //  note: the ptr null check is not strictly required
            if (m_live_allocation_count != 0u)
            {
                m_allocator->byte_deallocate(ptr, align);
                m_live_allocation_count -= 1u;
            }
        }
    }
}

inline std::uint32_t CAllocationContext::get_live_allocation_count() const noexcept
{
    return m_live_allocation_count;
}

inline std::uint32_t CAllocationContext::get_peak_allocation_count() const noexcept
{
    return m_peak_allocation_count;
}

inline std::uint64_t CAllocationContext::get_lifetime_allocation_count() const noexcept
{
    return m_lifetime_allocation_count;
}

}   //  namespace memory

#endif  //  #ifndef ALLOCATION_CONTEXT_HPP_INCLUDED
