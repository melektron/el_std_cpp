/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
03.12.23, 23:03
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Utilities for runtime type information (RTTI)
*/

#pragma once

#include <string>
#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace el::rtti
{
    inline std::string demangle_if_possible(const char* _typename)
    {

#ifdef __GNUC__
        int status = 0;
        char *ex_type_name = abi::__cxa_demangle(_typename, nullptr, nullptr, &status);
        std::string output(ex_type_name);
        free(ex_type_name); // required by cxxabi
        return output;
#else
        return _typename
#endif
    }
} // namespace el::rtti
