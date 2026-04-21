
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   TQueueEndpoints.hpp
//  Author: Ritchie Brannan
//  Date:   18 Apr 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Triple-buffered SPSC sequential transport endpoints for trivially copyable T.
//
//  IMPORTANT SEMANTIC NOTE
//  -----------------------
//  post() and read() are all-or-nothing.

#pragma once

#ifndef TQUEUE_ENDPOINTS_HPP_INCLUDED
#define TQUEUE_ENDPOINTS_HPP_INCLUDED

#include "containers/TPodVector.hpp"
#include "threading/transports/core/TQueueTransport.hpp"
#include "threading/transports/interfaces/TQueueInterfaces.hpp"
#include "debug/debug.hpp"

namespace threading::transports
{

//==============================================================================
//  TQueueProducerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) producer endpoint
//==============================================================================

template<typename T>
class TQueueProducerEndpoint : public TQueueProducerInterface<T>
{
public:
    explicit TQueueProducerEndpoint(TQueue<T>& queue) noexcept : m_queue{ &queue } {}
    ~TQueueProducerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool is_poisoned() const noexcept override final;
    [[nodiscard]] virtual bool post(const T& src) noexcept override final;
    [[nodiscard]] virtual bool post(const T* const src, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool post(const TPodConstView<T>& src) noexcept override final;
    [[nodiscard]] virtual bool post_would_reallocate(const std::uint32_t count) const noexcept override final;

private:
    TQueue<T>* m_queue = nullptr;
};

//==============================================================================
//  TQueueConsumerEndpoint<T>
//  Single Producer, Single Consumer (SPSC) consumer endpoint
//==============================================================================

template<typename T>
class TQueueConsumerEndpoint : public TQueueConsumerInterface<T>
{
public:
    explicit TQueueConsumerEndpoint(TQueue<T>& queue) noexcept : m_queue{ &queue } {}
    ~TQueueConsumerEndpoint() noexcept = default;

    //  Virtual overrides
    [[nodiscard]] virtual bool is_valid() const noexcept override final;
    [[nodiscard]] virtual bool is_ready() const noexcept override final;
    [[nodiscard]] virtual bool read(T& dst) noexcept override final;
    [[nodiscard]] virtual bool read(T* const dst, const std::uint32_t count = 1u) noexcept override final;
    [[nodiscard]] virtual bool read(const TPodView<T>& dst) noexcept override final;
    [[nodiscard]] virtual std::uint32_t current_readable_count() const noexcept override final;
    [[nodiscard]] virtual std::uint32_t refresh_readable_count() noexcept override final;

private:
    TQueue<T>* m_queue = nullptr;
};

//==============================================================================
//  TQueueProducerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TQueueProducerEndpoint<T>::is_valid() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->posting_is_valid() : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::is_ready() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->posting_is_ready() : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::is_poisoned() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->posting_poisoned() : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::post(const T& src) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->post(src) : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::post(const T* const src, const std::uint32_t count) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->post(src, count) : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::post(const TPodConstView<T>& src) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->post(src) : false;
}

template<typename T>
inline bool TQueueProducerEndpoint<T>::post_would_reallocate(const std::uint32_t count) const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->post_would_reallocate(count) : false;
}

//==============================================================================
//  TQueueConsumerEndpoint<T> out of class function bodies
//==============================================================================

template<typename T>
inline bool TQueueConsumerEndpoint<T>::is_valid() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->reading_is_valid() : false;
}

template<typename T>
inline bool TQueueConsumerEndpoint<T>::is_ready() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->reading_is_ready() : false;
}

template<typename T>
inline bool TQueueConsumerEndpoint<T>::read(T& dst) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->read(dst) : false;
}

template<typename T>
inline bool TQueueConsumerEndpoint<T>::read(T* const dst, const std::uint32_t count) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->read(dst, count) : false;
}

template<typename T>
inline bool TQueueConsumerEndpoint<T>::read(const TPodView<T>& dst) noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->read(dst) : false;
}

template<typename T>
inline std::uint32_t TQueueConsumerEndpoint<T>::current_readable_count() const noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->current_readable_count() : 0u;
}

template<typename T>
inline std::uint32_t TQueueConsumerEndpoint<T>::refresh_readable_count() noexcept
{
    MV_HARD_ASSERT(m_queue != nullptr);
    return (m_queue != nullptr) ? m_queue->refresh_readable_count() : 0u;
}

}   //  namespace threading::transports

#endif  //  #ifndef TQUEUE_ENDPOINTS_HPP_INCLUDED
