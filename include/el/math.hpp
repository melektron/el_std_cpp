/*
ELEKTRON © 2025 - now
Written by melektron
www.elektron.work
28.12.25, 23:31
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Mathematical utilities
*/

#pragma once

#include <stdint.h>

namespace el::math
{
    /**
     * @brief Clamps a value _v to not exceed the range defined
     * by values _min and _max such that _min <= ret_val <= _max. 
     * Works on any type that has > and < operators.
     * 
     * @tparam _T datatype to operate on (deducted)
     * @param _v input value
     * @param _min lower limit
     * @param _max upper limit
     * @return _T _v but restricted to the limits
     */
    template<typename _T>
    _T clamp(const _T& _v, const _T& _min, const _T& _max)
    {
        if (_v < _min) return _min;
        if (_v > _max) return _max;
        return _v;
    }
};