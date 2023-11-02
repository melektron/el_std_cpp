/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
02.11.23, 22:03
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink exceptions
*/

#pragma once

#include <websocketpp/error.hpp>
#include <el/exceptions.hpp>

namespace el::msglink
{
    class initialization_error : public el::exception
    {
        using el::exception::exception;
    };
    
    class launch_error : public el::exception
    {
        using el::exception::exception;
    };

    class serving_error : public el::exception
    {
        using el::exception::exception;
    };

    class termination_error: public el::exception
    {
        using el::exception::exception;
    };

} // namespace el::msglink
