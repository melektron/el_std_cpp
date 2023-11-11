/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
11.11.23, 23:00
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink event class used to define custom events
*/

#pragma once

#include <el/codable.hpp>


namespace el::msglink
{
    
#define EL_MSGLINK_DEFINE_EVENT(TypeName, ...)                                      \
    static inline const char *_event_name = #TypeName;                              \
                                                                                    \
    EL_DEFINE_CODABLE(TypeName, __VA_ARGS__)

    /**
     * @brief base class for all msglink event definition classes
     * 
     */
    struct event : public el::codable
    {
    };
} // namespace el::msglink
