/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
26.11.22, 18:25
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Preprocessor definitions for detecting C++ verisons
and enabeling supported features.

Currentyl this might not work for all compilers and it might not work at all.
In order to bypass all version checking, just 
#define __EL_ENABLE_CXX11
#define __EL_ENABLE_CXX17
to enable library features for versions not detected using the __cplusplus definition.
*/

#pragma once

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


#if (defined(__cpp_exceptions) && __cpp_exceptions == 199711L) || (defined(__EXCEPTIONS) && __EXCEPTIONS == 1)
// TODO: how to check this on MSVC?

#ifndef EL_DISABLE_EXCEPTIONS
#define __EL_ENABLE_EXCEPTIONS
#endif

#endif
