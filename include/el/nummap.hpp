/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
18.06.24, 00:07
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

numerical mapping functions
*/

#pragma once

#include "cxxversions.h"


namespace el
{
    /**
     * @brief linearly maps a number from range [_in_a .. _in_b] to
     * range [_out_a .. _out_b]. This may include inversions
     * in direction (e.g. mapping 1-4 to 4-1). The number 
     * does not have to be within and is not clamped to the specified ranges,
     * they merely serve as the two points required to identify a linear function.
     * 
     * @tparam _NT number type used (when using small integers, internally used division may cause unusable output)
     * @param _x number to map
     * @param _in_a first input value (that will be mapped to _out_a)
     * @param _in_b second input value (that will be mapped to _out_b)
     * @param _out_a first output value (mapped from _in_a)
     * @param _out_b second output value (mapped from _in_b)
     * @return _NT mapped number
     */
    template <typename _NT>
    _NT map_lin(
        _NT _x, 
        _NT _in_a,
        _NT _in_b,
        _NT _out_a,
        _NT _out_b
    ) {
        return (_x - _in_a) * (_out_b - _out_a) / (_in_b - _in_a) + _out_a;
    }
} // namespace el
