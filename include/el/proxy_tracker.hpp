/*
ELEKTRON © 2023
Written by Matteo Reiter
www.elektron.work
10.06.23, 23:29
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Class(es) to wrap data structures and variables in order to track changes
*/

#pragma once

#include <string.h>

#include "cxxversions.h"
#ifdef __EL_ENABLE_CXX11

namespace el
{
    template<class _T>
    class proxy_tracker
    {
    protected:
        // data_snapshot stores a snapshot to compare to, data_container is the
        // actual readable and writable data
        _T data_snapshot, data_container;
    
    public:
        _T *operator->()
        {
            return &data_container;
        }

        _T &operator*()
        {
            return data_container;
        }

        /**
         * @brief accepts any changes made to the container, saving them 
         * to the snapshots so any further compares to that data will be equal.
         */
        void accept()
        {
            memcpy(&data_snapshot, &data_container, sizeof(data_snapshot));
        }

        /**
         * @brief reverts any changes made to the container, going back to the values
         * of the previous snapshot
         */
        void revert()
        {
            memcpy(&data_container, &data_snapshot, sizeof(data_container));
        }

        /**
         * @brief compares the current value of a data member to the value of the
         * last snapshot. for this, a != operator must be implemented/deductible on the member types
         * use like this: myproxy.has_changed(&my_struct_type_t::my_struct_member)
         * 
         * @tparam _M type of the struct member (will be deducted automatically)
         * @param _member the member pointer (offset inside the structure)
         * @return true the data has changed since last accept
         * @return false the data has not changed
         * 
         * @note how to come up with this:
         * https://stackoverflow.com/questions/13180842/how-to-calculate-offset-of-a-class-member-at-compile-time
         * how these wired parameter types work:
         * https://stackoverflow.com/questions/6586205/what-are-the-pointer-to-member-operators-and-in-c
         */
        template <typename _M>  // member type
        bool has_changed(_M _T::*_member)
        {
            return data_container.*_member != data_snapshot.*_member;
        }

    };
} // namespace el


#endif