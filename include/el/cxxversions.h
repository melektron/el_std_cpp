/*
ELEKTRON © 2022 - now
Written by melektron
www.elektron.work
26.11.22, 18:25
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Preprocessor definitions for detecting C++ versions
and enabling supported features.

The library checks for the ENABLE macro definitions.

Currently this might not work for all compilers and it might not work at all.
In order to bypass all version checking, just 
#define __EL_ENABLE_CXX11
#define __EL_ENABLE_CXX17
#define __EL_ENABLE_CXX20
to enable library features for versions not detected using the __cplusplus definition.
*/

#pragma once

#ifdef __linux__
#define __EL_PLATFORM_LINUX
#endif
#ifdef __APPLE__
#define __EL_PLATFORM_APPLE
#endif
#ifdef _WIN32
#define __EL_PLATFORM_WINDOWS
#endif

// check for C++ 11 compatablility
#if __cplusplus > 199711L

#define __EL_CXX11
#define __EL_ENABLE_CXX11

#endif

// check for C++ 14 compatability
#if __cplusplus >= 201402L

#define __EL_CXX14
#define __EL_ENABLE_CXX14

#endif

// check for C++ 17 compatability
#if __cplusplus >= 201703L

#define __EL_CXX17
#define __EL_ENABLE_CXX17

#endif

// check for C++ 20 compatibility
#if __cplusplus >= 202002L

#define __EL_CXX20
#define __EL_ENABLE_CXX20

#endif