/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
03.11.23, 14:13
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Includes and environment configuration for the websocket++ classes
needed by the msglink implementation
*/

#pragma once

#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>


namespace el::msglink
{
    // typedefs and namespaces to make code more readably
    namespace wspp = websocketpp;
    namespace pl = std::placeholders;
    typedef wspp::server<wspp::config::asio> wsserver;

}   // namespace el::msglink