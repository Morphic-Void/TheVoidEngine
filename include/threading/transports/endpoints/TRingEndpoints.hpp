
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TRingEndpoints.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed-capacity SPSC ring transport endpoints for trivially copyable T.
//
//  IMPORTANT SEMANTIC NOTE
//  -----------------------
//  post() and read() are all-or-nothing.

#pragma once

#ifndef TRING_ENDPOINTS_HPP_INCLUDED
#define TRING_ENDPOINTS_HPP_INCLUDED

#include "containers/TPodVector.hpp"
#include "threading/transports/core/TRingTransport.hpp"
#include "threading/transports/interfaces/TRingInterfaces.hpp"
#include "debug/debug.hpp"

namespace threading::transports
{

//==============================================================================
//  TRingProducerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) producer endpoint
//==============================================================================

template<typename T>
class TRingProducerEndpoint : public TRingProducerInterface<T>
{
public:
    explicit TRingProducerEndpoint(TRing<T>& ring) noexcept : m_ring{ &ring } {}
    ~TRingProducerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool post(const T& src) noexcept override final;
    [[nodiscard]] virtual bool post(const T* const src, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool post(const TPodConstView<T>& src) noexcept override final;
    [[nodiscard]] virtual std::uint32_t writable_count() const noexcept override final;

private:
    TRing<T>* m_ring = nullptr;
};

//==============================================================================
//  TRingConsumerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) consumer endpoint
//==============================================================================

template<typename T>
class TRingConsumerEndpoint : public TRingConsumerInterface<T>
{
public:
    explicit TRingConsumerEndpoint(TRing<T>& ring) noexcept : m_ring{ &ring } {}
    ~TRingConsumerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool read(T& dst) noexcept override final;
    [[nodiscard]] virtual bool read(T* const dst, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool read(const TPodView<T>& dst) noexcept override final;
    [[nodiscard]] virtual std::uint32_t readable_count() const noexcept override final;

private:
    TRing<T>* m_ring = nullptr;
};

//==============================================================================
//  TRingProducerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TRingProducerEndpoint<T>::is_valid() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->producer_is_valid() : false;
}

template<typename T>
inline bool TRingProducerEndpoint<T>::is_ready() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->is_ready() : false;
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const T& src) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->post(src) : false;
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const T* const src, const std::uint32_t count) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->post(src, count) : false;
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const TPodConstView<T>& src) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->post(src) : false;
}

template<typename T>
inline std::uint32_t TRingProducerEndpoint<T>::writable_count() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->writable_count() : 0u;
}

//==============================================================================
//  TRingConsumerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TRingConsumerEndpoint<T>::is_valid() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->consumer_is_valid() : false;
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::is_ready() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->is_ready() : false;
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(T& dst) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->read(dst) : false;
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(T* const dst, const std::uint32_t count) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->read(dst, count) : false;
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(const TPodView<T>& dst) noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->read(dst) : false;
}

template<typename T>
inline std::uint32_t TRingConsumerEndpoint<T>::readable_count() const noexcept
{
    MV_HARD_ASSERT(m_ring != nullptr);
    return (m_ring != nullptr) ? m_ring->readable_count() : 0u;
}

}   //  namespace threading::transports

#endif  //  #ifndef TRING_ENDPOINTS_HPP_INCLUDED

