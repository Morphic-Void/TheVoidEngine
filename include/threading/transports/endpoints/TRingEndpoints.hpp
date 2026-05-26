
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
#include "threading/transports/TRingTransport.hpp"
#include "threading/transports/interfaces/TRingInterfaces.hpp"

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
    explicit TRingProducerEndpoint(TRing<T>& ring) noexcept : m_ring{ ring } {}
    ~TRingProducerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool post(const T& src) noexcept override final;
    [[nodiscard]] virtual bool post(const T* const src, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool post(const TPodConstView<T>& src) noexcept override final;
    [[nodiscard]] virtual std::uint32_t writable_count() const noexcept override final;

private:
    TRing<T>& m_ring;
};

//==============================================================================
//  TRingConsumerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) consumer endpoint
//==============================================================================

template<typename T>
class TRingConsumerEndpoint : public TRingConsumerInterface<T>
{
public:
    explicit TRingConsumerEndpoint(TRing<T>& ring) noexcept : m_ring{ ring } {}
    ~TRingConsumerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool read(T& dst) noexcept override final;
    [[nodiscard]] virtual bool read(T* const dst, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool read(const TPodView<T>& dst) noexcept override final;
    [[nodiscard]] virtual std::uint32_t readable_count() const noexcept override final;

private:
    TRing<T>& m_ring;
};

//==============================================================================
//  TRingProducerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TRingProducerEndpoint<T>::is_valid() const noexcept
{
    return m_ring.posting_is_valid();
}

template<typename T>
inline bool TRingProducerEndpoint<T>::is_ready() const noexcept
{
    return m_ring.is_ready();
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const T& src) noexcept
{
    return m_ring.post(src);
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const T* const src, const std::uint32_t count) noexcept
{
    return m_ring.post(src, count);
}

template<typename T>
inline bool TRingProducerEndpoint<T>::post(const TPodConstView<T>& src) noexcept
{
    return m_ring.post(src);
}

template<typename T>
inline std::uint32_t TRingProducerEndpoint<T>::writable_count() const noexcept
{
    return m_ring.writable_count();
}

//==============================================================================
//  TRingConsumerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TRingConsumerEndpoint<T>::is_valid() const noexcept
{
    return m_ring.reading_is_valid();
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::is_ready() const noexcept
{
    return m_ring.is_ready();
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(T& dst) noexcept
{
    return m_ring.read(dst);
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(T* const dst, const std::uint32_t count) noexcept
{
    return m_ring.read(dst, count);
}

template<typename T>
inline bool TRingConsumerEndpoint<T>::read(const TPodView<T>& dst) noexcept
{
    return m_ring.read(dst);
}

template<typename T>
inline std::uint32_t TRingConsumerEndpoint<T>::readable_count() const noexcept
{
    return m_ring.readable_count();
}

}   //  namespace threading::transports

#endif  //  #ifndef TRING_ENDPOINTS_HPP_INCLUDED
