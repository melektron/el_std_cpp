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

    struct wspp_config : public wspp::config::asio
    {
        typedef wspp_config type;   // apply standard names for what is this config type
        typedef asio base;          // standard name for what this config is based on (the default asio config)

        typedef base::concurrency_type concurrency_type;

        typedef base::request_type request_type;
        typedef base::response_type response_type;

        typedef base::message_type message_type;
        typedef base::con_msg_manager_type con_msg_manager_type;
        typedef base::endpoint_msg_manager_type endpoint_msg_manager_type;

        typedef base::alog_type alog_type;
        typedef base::elog_type elog_type;

        typedef base::rng_type rng_type;

        struct transport_config : public base::transport_config {
            typedef type::concurrency_type concurrency_type;
            typedef type::alog_type alog_type;
            typedef type::elog_type elog_type;
            typedef type::request_type request_type;
            typedef type::response_type response_type;
            typedef websocketpp::transport::asio::basic_socket::endpoint
                socket_type;
        };

        typedef websocketpp::transport::asio::endpoint<transport_config>
            transport_type;
    };

    typedef wspp::server<wspp_config> wsserver;

}   // namespace el::msglink