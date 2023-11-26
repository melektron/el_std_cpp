/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
26.11.23, 15:26
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

flag types with specific restrictions
*/

#pragma once

#include "cxxversions.h"


namespace el
{
    using flag = bool;

    /**
     * @brief set-only flag.
     * 
     * This single use flag behaves like a normal boolean flag,
     * except it can only be set but never cleared again.
     * When default initialized, the flag is initialized to false.
     * When copy/move initialized, the flag is initialized as expected.
     */
    class soflag
    {
    protected:
        flag internal_flag = false;
    
    public:
        // allow default init
        soflag() = default;
        soflag(const soflag &) = default;
        soflag(soflag &&) = default;

        // cannot copy assign as that would allow clearing
        soflag &operator=(const soflag &) = delete;

        /**
         * @brief sets the set only flag to the value of 
         * the provided flag if it is set. If the provided
         * flag _x is cleared, nothing will happen.
         * 
         * @param _x whether to set
         * @return flag& value of the soflag after the operation (not _x)
         */
        inline flag &operator=(flag _x) noexcept
        {
            if (_x)
                internal_flag = _x;
            
            return internal_flag;
        }

        /**
         * @brief sets the flag.
         * If already set, nothing happens.
         */
        inline void set() noexcept
        {
            internal_flag = true;
        }

        /**
         * @return flag the current value of the flag (set/cleared)
         */
        inline operator flag() const noexcept
        {
            return internal_flag;
        }
    };

} // namespace el
