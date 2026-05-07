
//  Copyright (c) 2026 Ritchie Brannan / Morphic Void Limited
//  License: MIT (see LICENSE file in repository root)
//
//  File:   platform_defines.hpp
//  Author: Ritchie Brannan
//  Date:   7 May 26
//
//  Requirements:
//  - Requires C++17 or later.
//  - No exceptions.
//
//  Platform selection defines for low-level platform implementation files.
//
//  This header only classifies the target platform. It does not include
//  platform headers, declare platform APIs, or provide feature wrappers.

#pragma once

#ifndef PLATFORM_DEFINES_HPP_INCLUDED
#define PLATFORM_DEFINES_HPP_INCLUDED

#if defined(_WIN32) || defined(_WIN64)
#define MV_PLATFORM_WINDOWS
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define MV_PLATFORM_APPLE
#endif

#if defined(__linux__) && !defined(__ANDROID__)
#define MV_PLATFORM_LINUX_ONLY
#endif

#if defined(__ANDROID__)
#define MV_PLATFORM_ANDROID_ONLY
#endif

#if defined(__linux__) || defined(__ANDROID__)
#define MV_PLATFORM_LINUX_ANDROID
#endif

#if defined(MV_PLATFORM_APPLE) || defined(MV_PLATFORM_LINUX_ANDROID)
#define MV_PLATFORM_HAS_PTHREADS
#endif

#endif  //  #ifndef PLATFORM_DEFINES_HPP_INCLUDED
