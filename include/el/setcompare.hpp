/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
26.07.24, 14:22
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

comparison utilities for lazy static sets of objects
*/

#pragma once

#include <initializer_list>

#include "cxxversions.h"

#ifdef __EL_ENABLE_CXX11

namespace el
{
    /**
     * @brief comparison set to logically compare
     * with ANY of the provided elements.
     * 
     * @tparam _T value type
     */
    template<typename _T>
    class any_of
    {
        std::initializer_list<_T> items;
    public:
        any_of(std::initializer_list<_T> _items)
            : items(_items)
        {}

        /**
         * @brief Tests if the value equals
         * any of the provided elements.
         * @note see definition for example
         * 
         * @example
         * ```cpp
         * assert( (el::any_of{2, 3, 4, 5, 6} == 5) );
         * assert( (el::any_of{2, 3, 4, 5, 6} == 8) ); // assert failed
         * ```
         * 
         * @return true the value equals at least one of the elements in the set
         * @return false the value equals none of teh elements in the set
         */
        bool operator==(const _T &_rhs) const
        {
            for (const _T &item : items)
            {
                if (item == _rhs)
                    return true;
            }
            return false;
        }

        /**
         * @brief Tests if the value doesn't equals
         * any of the provided elements.
         * @note see definition for example
         * 
         * @example
         * ```cpp
         * assert( (el::any_of{2, 3, 4, 5, 6} == 8) );
         * assert( (el::any_of{2, 3, 4, 5, 6} == 5) ); // assert failed
         * ```
         * 
         * @return true the value equals at least one of the elements in the set
         * @return false the value equals none of the elements in the set
         */
        bool operator!=(const _T &_rhs) const
        {
            for (const _T &item : items)
            {
                if (item == _rhs)
                    return false;
            }
            return true;
        }
    };
};

/**
 * @brief Tests if the value equals
 * any of the provided elements.
 * @note see definition for example
 * 
 * @example
 * ```cpp
 * assert( (5 == el::any_of{2, 3, 4, 5, 6}) );
 * assert( (8 == el::any_of{2, 3, 4, 5, 6}) ); // assert failed
 * ```
 * 
 * @return true the value equals at least one of the elements in the set
 * @return false the value equals none of the elements in the set
 */
template<typename _T>
bool operator==(const _T &_lhs, const el::any_of<_T>& _rhs)
{
    return _rhs == _lhs;
}

/**
 * @brief Tests if the value doesn't equals
 * any of the provided elements.
 * @note see definition for example
 * 
 * @example
 * ```cpp
 * assert( (8 == el::any_of{2, 3, 4, 5, 6}) );
 * assert( (5 == el::any_of{2, 3, 4, 5, 6}) ); // assert failed
 * ```
 * 
 * @return true the value equals at least one of the elements in the set
 * @return false the value equals none of teh elements in the set
 */
template<typename _T>
bool operator!=(const _T &_lhs, const el::any_of<_T>& _rhs)
{
    return _rhs != _lhs;
}

#endif  // __EL_ENABLE_CXX11