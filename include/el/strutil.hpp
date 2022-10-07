/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
07.10.22, 21:28
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Utility functions operating for strings, mostly STL compatible string types.
*/

#pragma once

#include <stdio.h>
#include <string>
#include <memory>

namespace el::strutil
{
    /**
     * @brief creates a format string like printf or s(n)printf using the system's
     * snprintf function wrapped in a C++-like interface. 
     * Any STL compatible string type can be used as the format and return value type (see _ST).
     * NOTE: This method uses dynamic memory allocation ("new" operator) and std::unique_ptr.
     * Usually, the template arguments don't have to be provided but can be deducted from function
     * arguments.
     * 
     * @tparam _ST string type of the format and return value. The type must be constructable
     * from "const char *" (string copy) and must have a .c_str() method to convert to "char *"".
     * @tparam _Args varadic format argument types
     * @param _fmt Format string
     * @param _args Format arguments
     * @return _ST newly created string of specified type
     */
    template<typename _ST, typename... _Args>
    _ST format(const _ST& _fmt, _Args... _args)
    {
        size_t len = snprintf(nullptr, 0, _fmt.c_str(), _args...) + 1;  // extra space for null byte
        std::unique_ptr<char[]> _cstr(new char[len]);
        snprintf(_cstr.get(), len, _fmt.c_str(), _args...);
        return _ST(_cstr.get());
    }
};