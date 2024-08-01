/*
ELEKTRON © 2024
Written by Matteo Reiter
www.elektron.work
01.08.24, 15:19
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Utility functions for C strings
*/

#pragma once

#include <stdio.h>
#include <string.h>

namespace el::cstr
{
    /**
     * @brief copies a null-terminated C string from _src to _dest while 
     * ensuring _dest is null-terminated and ensuring that the boundaries
     * of the _dest buffer are not exceeded, and additionally returns
     * the length of the output string. This is a full C implementation of
     * a copy which also performs a strlen at the cost of some performance.
     * 
     * Characters are copied until the null terminator of _src is reached
     * or the number of characters copied is _n-1. In any case, a null terminator
     * is added to the destination string, except for when the destination buffer
     * size _n is zero, in which case _dest is not modified.
     * 
     * @tparam _CT the character type. Typically char, but any other integral type (e.g. wchar) is also supported.
     * @param _dest pointer to the destination buffer to store the string
     * @param _src pointer to a null-terminated source string.
     * @param _n size of the _dest buffer, including the space for the null terminator.
     * @return size_t the string length (not including the null terminator)
     * of the destination string after copy (equivalent to strlen(_dest)). 
     * This will be at most _n-1 (because the null termination byte is always 
     * added) or strlen(_src), whichever is greater.
     */
    template<typename _CT = char>
    inline size_t copy(_CT *_dest, const _CT *_src, size_t _n)
    {
        // if buffer size is zero, do nothing
        if (_n == 0)
            return 0;
        
        // copy characters while the number of occupied characters (c)
        // is smaller than the available space (_n)
        size_t c = 0;
        while (++c < _n && *_src != '\0')
            *(_dest++) = *(_src++); // copy and then increment

        // add null terminator to end
        *(_dest) = '\0';

        return c - 1;   // remove null terminator from count
    }


    /**
     * @brief copies a null-terminated C string from _src to _dest while 
     * ensuring _dest is null-terminated and ensuring that the boundaries
     * of the _dest buffer are not exceeded. Basically, this is the behaviour
     * you probably want when using strncpy.
     * 
     * Characters are copied until the null terminator of _src is reached
     * or the number of characters copied is _n-1. In any case, a null terminator
     * is added to the destination string, except for when the destination buffer
     * size _n is zero, in which case _dest is not modified.
     * 
     * @param _dest pointer to the destination buffer to store the string
     * @param _src pointer to a null-terminated source string.
     * @param _n size of the _dest buffer, including the space for the null terminator.
     * @return _dest
     */
    inline char *strntcpy(char *_dest, const char *_src, size_t _n)
    {
        // if buffer size is zero, do nothing
        if (_n == 0)
            return 0;
        
        // copy characters 
        strncpy(_dest, _src, _n);

        // add null terminator to end
        *(_dest + _n - 1) = '\0';

        return _dest;
    }


};