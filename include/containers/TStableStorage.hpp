
//  File:   TStableStorage.hpp
//  Author: Ritchie Brannan
//  Date:   24 Mar 26
//

#pragma once

#ifndef TSTABLE_STORAGE_HPP_INCLUDED
#define TSTABLE_STORAGE_HPP_INCLUDED

#include <cstddef>      //  std::size_t
#include <type_traits>  //  std::is_const_v

#include "memory/memory_allocation.hpp"
#include "memory/memory_primitives.hpp"
#include "debug/debug.hpp"

//==============================================================================
//  TStableStorage<T>
//  Owning unique stable raw allocation buffers for typed data.
//==============================================================================

template<typename T>
class TStableStorage
{
private:
    static_assert(!std::is_const_v<T>, "TStableStorage<T> requires non-const T.");

public:

    //  Default and deleted lifetime
    TStableStorage() noexcept = default;
    TStableStorage(const TStableStorage&) noexcept = delete;
    TStableStorage& operator=(const TStableStorage&) noexcept = delete;
    TStableStorage(TStableStorage&&) noexcept = default;
    TStableStorage& operator=(TStableStorage&&) noexcept = default;

    //  Destructor
    ~TStableStorage() noexcept { deallocate(); };

    //  Status
    [[nodiscard]] bool is_valid() const noexcept;
    [[nodiscard]] bool is_empty() const noexcept;
    [[nodiscard]] bool is_ready() const noexcept;

    //  Accessors
    [[nodiscard]] T* map_index(const std::size_t slot_index) noexcept;
    [[nodiscard]] T* index_ptr(const std::size_t slot_index) const noexcept;

    //  Initialisation and deallocation
    bool initialise(const std::size_t slots_per_buffer) noexcept;
    void deallocate() noexcept;

    //  Constants
    static constexpr std::size_t k_max_elements = memory::t_max_elements<T>();
    static constexpr std::size_t k_element_size = sizeof(T);

private:
    [[nodiscard]] std::size_t buffer_count() const noexcept { return m_slot_capacity >> m_buffer_shift; }
    [[nodiscard]] std::size_t buffer_slots() const noexcept { return m_slot_mask + 1u; }
    [[nodiscard]] std::size_t buffer_index(const std::size_t slot_index) const noexcept { return slot_index >> m_buffer_shift; }
    [[nodiscard]] std::size_t buffer_slot(const std::size_t slot_index) const noexcept { return slot_index & m_slot_mask; }

    memory::TMemoryToken<memory::TMemoryToken<T>> m_buffers = memory::TMemoryToken<memory::TMemoryToken<T>>{};

    std::size_t m_buffer_capacity = 0u;
    std::size_t m_buffer_shift = 0u;
    std::size_t m_slot_mask = 0u;
    std::size_t m_slot_capacity = 0u;
};

template<typename T>
inline bool TStableStorage<T>::is_valid() const noexcept
{
    //if (m_buffer_ptrs.data() == nullptr)
    //{
    //    return (m_buffer_ptr_capacity | m_buffer_shift | m_slot_mask | m_slot_capacity) == 0u;
    //}

    //TBD
    //m_buffer_ptr_capacity <- must be pow2 (0 is not pow2)
    //m_slot_mask <- (1 << m_buffer_shift) -1
    //m_slot_capacity <- must be multiple of (m_slot_mask + 1u), must be less than or equal to m_buffer_ptr_capacity * (m_slot_mask + 1u)
}

template<typename T>
inline bool TStableStorage<T>::is_empty() const noexcept
{
    //TBD
}

template<typename T>
inline bool TStableStorage<T>::is_ready() const noexcept
{
    //TBD
}

template<typename T>
inline T* TStableStorage<T>::map_index(const std::size_t slot_index) noexcept
{
    if (is_ready() && (slot_index < k_max_elements))
    {
        while (slot_index >= m_slot_capacity)
        {   //  allocate additional buffers to fulfil the request
            const std::size_t buffer_index = buffer_count()
            if (buffer_index == m_buffer_capacity)
            {   //  need to grow the buffer directory
                if (!m_buffers.reallocate(m_buffer_capacity, (m_buffer_capacity << 1)))
                {   //  reallocation failed
                    return nullptr;
                }
                m_buffer_capacity <<= 1;
            }
            if (!m_buffers.data()[buffer_index].allocate(buffer_slots()))
            {   //  allocation failed
                return nullptr;
            }
            m_slot_capacity += buffer_slots();
        }
        return &m_buffers.data()[buffer_index(slot_index)].data()[buffer_slot(slot_index)];
    }
    return nullptr;
}

template<typename T>
inline T* TStableStorage<T>::index_ptr(const std::size_t slot_index) const noexcept
{
    if (is_ready() && (index < m_slot_capacity))
    {
        return &m_buffers.data()[buffer_index(slot_index)].data()[buffer_slot(slot_index)];
    }
    return nullptr;
}

template<typename T>
inline bool TStableStorage<T>::initialise(const std::size_t slots_per_buffer) noexcept
{
    //TBD
}

template<typename T>
inline void TStableStorage<T>::deallocate() noexcept
{
    memory::TMemoryToken<T>* buffers = m_buffer.data();
    if (buffers != nullptr)
    {
        std::size_t count = get_buffer_count();
        while (count)
        {
            --count;
            if (buffers[count].data() != nullptr)
            {
                buffers[count].deallocate();
            }
        }
        m_buffers.deallocate();
    }
    m_buffer_ptr_capacity = 0u;
    m_buffer_shift = 0u;
    m_slot_mask = 0u;
    m_slot_capacity = 0u;
}

#endif  //  TSTABLE_STORAGE_HPP_INCLUDED
