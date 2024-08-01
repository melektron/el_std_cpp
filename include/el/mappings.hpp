/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
31.07.24, 16:07
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

mapping functions to convert between values or sets of values
*/

#pragma once

#include <initializer_list>

#include "cxxversions.h"

#ifdef __EL_ENABLE_CXX11

namespace el
{
    template<typename _TF, typename _TT, std::size_t _N>
    class ref_set_mapping
    {
        const _TF (&from)[_N];
        const _TT (&to)[_N];
        const _TT &default_val;

    public:
        /**
         * @brief Construct a new ref set mapping.
         * This mapping holds a reference to the two provided sets of values
         * and allows mapping a value from one set to the value in the other
         * set at the equivalent index by calling it with the value to map.
         * 
         * 
         * @param _from set of possible input values
         * @param _to set of corresponding output values (must be same size as input values)
         * @param _default_val value to return when input value is not found in input set
         * 
         * @attention this object doesn't take ownership or copy any provided values.
         * The response from the mapping has the same scope as provided output set.
         * This is intended for inline use only, without storing the ref_set_mapping object
         * for prolonged periods of time.
         * 
         * @note this is flagged for needing C++11, however it is advised to use it
         * with C++17 as template arguments would otherwise have to be explicitly
         * provided.
         */
        constexpr ref_set_mapping(const _TF (&_from)[_N], const _TT (&_to)[_N], const _TT &_default_val)
            : from(_from)
            , to(_to)
            , default_val(_default_val)
        {}

        /**
         * @brief maps the _value_to_map to the value at the equivalent
         * index in the _to set.
         * 
         * @param _value_to_map 
         * @return _TT& reference to the value in the _to set
         * 
         * @attention the reference is only valid as long as the array is
         * (typically in the local scope)
         */
        constexpr const _TT &operator()(const _TF &_value_to_map) const
        {
            for (std::size_t i = 0; i < _N; i++)
            {
                if (from[i] == _value_to_map)
                    return to[i];
            }
            return default_val;
        }
        
    };
    
};

#endif  // __EL_ENABLE_CXX11