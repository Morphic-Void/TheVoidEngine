
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   type_ids.hpp
//  Author: Ritchie Brannan
//  Date:   23 Feb 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Fixed encoded type ids for closed-system use across DLLs.
//
//  Provides encode/decode/validation helpers and project-level
//  id constants.
//
//  This file defines ids only. It does not bind ids to C++ types.

#pragma once

#ifndef TYPE_IDS_HPP_INCLUDED
#define TYPE_IDS_HPP_INCLUDED

#include <cstddef>      //  std::size_t

namespace type_ids
{

constexpr std::size_t encode_type_id(const std::size_t value) noexcept
{
	if (value >= 0x0000ffffu)
	{
		return std::size_t{ 0u };
	}
	std::size_t t = value;
	t ^= 0x0000ffffu;
	t = ((t << 8) | t) & 0x00ff00ffu;
	t = ((t << 4) | t) & 0x0f0f0f0fu;
	t = ((t << 2) | t) & 0x33333333u;
	t = ((t << 1) | t) & 0x55555555u;
	return t;
}

constexpr std::size_t decode_type_id(const std::size_t type_id) noexcept
{
	if ((type_id & 0xaaaaaaaau) != 0u)
	{
		return 0u;
	}
	std::size_t t = type_id;
	t = ((t >> 1) | t) & 0x33333333u;
	t = ((t >> 2) | t) & 0x0f0f0f0fu;
	t = ((t >> 4) | t) & 0x00ff00ffu;
	t = ((t >> 8) | t) & 0x0000ffffu;
	t ^= 0x0000ffffu;
	return t;
}

constexpr bool is_valid_type_id(const std::size_t type_id) noexcept
{
	return decode_type_id(type_id) != 0u;
}

constexpr std::size_t byte_buffer      = encode_type_id(1u);
constexpr std::size_t byte_rect_buffer = encode_type_id(2u);
constexpr std::size_t simple_string    = encode_type_id(3u);
constexpr std::size_t string_buffer    = encode_type_id(4u);
constexpr std::size_t stable_strings   = encode_type_id(5u);

}   //  namespace type_ids

#endif  //  #ifndef TYPE_IDS_HPP_INCLUDED