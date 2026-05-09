
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   alternating_parking_gate.hpp
//  Author: Ritchie Brannan
//  Date:   9 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Alternating parking gate built from two exclusive locks.
//
//  CAlternatingParkingGate provides a small fallback parking primitive for
//  higher-level threading objects that need an owned place to block waiters.
//  The controller closes one phase while the other remains open. Parkers pass
//  through the phase recorded in their caller-owned CParkingTicket, then flip
//  the ticket for the next park.
//
//  Construction leaves both gates open. acquire_control() must be called by
//  the controller thread before controlled use. release_control() opens the
//  closed gate and returns the object to its uncontrolled state.
//
//  Control operations are single-controller per instance. Parking tickets are
//  caller-owned, backend state; each concurrently parking thread must use a
//  distinct ticket.

#pragma once

#ifndef ALTERNATING_PARKING_GATE_HPP_INCLUDED
#define ALTERNATING_PARKING_GATE_HPP_INCLUDED

#include <cstdint>  //  std::uint32_t

#include "platform/threading/exclusive_lock.hpp"

namespace threading
{

//==============================================================================
//  CParkingTicket
//==============================================================================

class CParkingTicket
{
public:

    //  Default and deleted lifetime
    CParkingTicket() noexcept = default;
    CParkingTicket(const CParkingTicket&) = delete;
    CParkingTicket& operator=(const CParkingTicket&) = delete;
    CParkingTicket(CParkingTicket&&) = delete;
    CParkingTicket& operator=(CParkingTicket&&) = delete;
    ~CParkingTicket() noexcept = default;

private:

    friend class CAlternatingParkingGate;

    std::uint32_t m_phase = 0u;
};

//==============================================================================
//  CAlternatingParkingGate
//==============================================================================

class CAlternatingParkingGate
{
public:

    //  Constructor and destructor
    CAlternatingParkingGate() noexcept;
    ~CAlternatingParkingGate() noexcept;

    //  Deleted lifetime
    CAlternatingParkingGate(const CAlternatingParkingGate&) = delete;
    CAlternatingParkingGate& operator=(const CAlternatingParkingGate&) = delete;
    CAlternatingParkingGate(CAlternatingParkingGate&&) = delete;
    CAlternatingParkingGate& operator=(CAlternatingParkingGate&&) = delete;

    //  Status
    bool is_valid() const noexcept;
    bool has_control() const noexcept;

    //  Control
    bool acquire_control() noexcept;
    void release_control() noexcept;

    //  Parking
    void park(CParkingTicket& ticket) noexcept;

    //  Phase control
    void cycle_phase() noexcept;

private:

    platform::threading::CExclusiveLock m_gates[2];

    std::uint32_t m_closed_phase = 0u;

    bool m_valid = false;
    bool m_has_control = false;
};

}   //  namespace threading

#endif // ALTERNATING_PARKING_GATE_HPP_INCLUDED
