
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   CParkingGate.cpp
//  Author: Ritchie Brannan
//  Date:   9 May 26

#include "threading/CParkingGate.hpp"
#include "platform/threading/processor_relax.hpp"
#include "debug/debug.hpp"

namespace threading
{

//==============================================================================
//  Constructor and destructor
//==============================================================================

CParkingGate::CParkingGate() noexcept
{
    m_valid = m_gates[0].is_valid() && m_gates[1].is_valid();
    m_closed_phase = 0u;
    m_has_control = false;
}

CParkingGate::~CParkingGate() noexcept
{
    MV_HARD_ASSERT(!m_has_control);
}

//==============================================================================
//  Status
//==============================================================================

bool CParkingGate::is_valid() const noexcept
{
    return m_valid;
}

bool CParkingGate::has_control() const noexcept
{
    return m_has_control;
}

//==============================================================================
//  Control
//==============================================================================

bool CParkingGate::acquire_control() noexcept
{
    if (MV_FAIL_SAFE_ASSERT(m_valid && !m_has_control))
    {
        m_closed_phase = 0u;
        m_gates[m_closed_phase].acquire();
        m_has_control = true;
        return true;
    }
    return false;
}

void CParkingGate::release_control() noexcept
{
    if (MV_FAIL_SAFE_ASSERT(m_valid && m_has_control))
    {
        m_gates[m_closed_phase].release();
        m_closed_phase = 0u;
        m_has_control = false;
    }
}

//==============================================================================
//  Parking
//==============================================================================

void CParkingGate::park(CParkingTicket& ticket) noexcept
{
    if (MV_FAIL_SAFE_ASSERT(m_valid))
    {
        const std::uint32_t phase = ticket.m_phase & 1u;
        platform::threading::processor_relax();
        m_gates[phase].acquire();
        m_gates[phase].release();
        ticket.m_phase = phase ^ 1u;
    }
}

//==============================================================================
//  Phase control
//==============================================================================

void CParkingGate::cycle_phase() noexcept
{
    if (MV_FAIL_SAFE_ASSERT(m_valid && m_has_control))
    {   //  Close the open phase before opening the closed phase.
        const std::uint32_t closed_phase = m_closed_phase;
        const std::uint32_t open_phase = closed_phase ^ 1u;
        m_gates[open_phase].acquire();
        m_gates[closed_phase].release();
        m_closed_phase = open_phase;
    }
}

}   //  namespace threading
