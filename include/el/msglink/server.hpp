/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
02.11.23, 22:02
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink server class
*/

#pragma once

#define ASIO_STANDALONE

#include <thread>
#include <atomic>
#include <map>
#include <functional>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include <el/retcode.hpp>
#include <el/logging.hpp>

#include <el/msglink/errors.hpp>


namespace el::msglink
{

    // typedefs and namespaces to make code more readably
    namespace wspp = websocketpp;
    namespace pl = std::placeholders;
    typedef wspp::server<wspp::config::asio> wsserver;

    class connection_handler
    {
        void on_open(wspp::connection_hdl hdl) noexcept;
        void on_http(wspp::connection_hdl hdl) noexcept;
        void on_message(wspp::connection_hdl hdl, wsserver::message_ptr msg) noexcept;
        void on_fail(wspp::connection_hdl hdl) noexcept;
        void on_close(wspp::connection_hdl hdl) noexcept;
    };

    class server
    {
    private:
        // == Configuration 
        // port to serve on
        int m_port;

        // == State
        // the websocket server used for transport
        wsserver m_socket_server;

        // enumeration managing current server state
        enum server_state_t
        {
            UNINITIALIZED = 0,  // newly instantiated, not initialized
            INITIALIZED = 1,    // initialize() successful
            RUNNING = 2,        // run() called, server still running
            FAILED = 3,         // run() exited with error
            STOPPED = 4         // run() exited cleanly (through stop() or other natural way)
        };
        server_state_t server_state = UNINITIALIZED;

        // set of connections to corresponding connection handler instance
        std::map<
            wspp::connection_hdl,
            connection_handler,
            std::owner_less<wspp::connection_hdl>
        > open_connections;

    private:

        /**
         * @brief websocket server callback functions called when 
         * new client connection is opened, a message is received, an error occurs
         * or the connection is closed.
         *
         * @param hdl websocket connection handle (a generic argument that identifies the connection, always present)
         * @param ... more arguments may be required for a specific function
         */
        void on_open(wspp::connection_hdl hdl) noexcept
        {
            std::cout << __FUNCTION__ << std::endl;
        }
        void on_http(wspp::connection_hdl hdl) noexcept
        {
            std::cout << __FUNCTION__ << std::endl;
        }
        void on_message(wspp::connection_hdl hdl, wsserver::message_ptr msg) noexcept
        {
            std::cout << __FUNCTION__ << std::endl;
        }
        void on_fail(wspp::connection_hdl hdl) noexcept
        {
            std::cout << __FUNCTION__ << std::endl;
        }
        void on_close(wspp::connection_hdl hdl) noexcept
        {
            std::cout << __FUNCTION__ << std::endl;
        }

    public:
        server()
            : m_port(8080)
        {}

        server(int _port)
            : m_port(_port)
        {}

        // standard init/terminate methods to start stop global object server in main
        void initialize()
        {
            if (server_state != UNINITIALIZED)
                throw initialization_error("msglink server instance is single use, cannot re-initialize");
            
            try
            {
                // we don't want any wspp log messages
                m_socket_server.clear_access_channels(wspp::log::alevel::all);
                m_socket_server.clear_error_channels(wspp::log::elevel::all);

                // initialize asio communication
                m_socket_server.init_asio();

                // register callback handlers
                m_socket_server.set_open_handler(std::bind(&server::on_open, this, pl::_1));
                m_socket_server.set_http_handler(std::bind(&server::on_http, this, pl::_1));
                m_socket_server.set_message_handler(std::bind(&server::on_message, this, pl::_1, pl::_2));
                m_socket_server.set_fail_handler(std::bind(&server::on_fail, this, pl::_1));
                m_socket_server.set_close_handler(std::bind(&server::on_close, this, pl::_1));

                // set reuse addr flag to allow faster restart times
                m_socket_server.set_reuse_addr(true);

            }
            catch (const std::exception &e)
            {
                throw initialization_error(el::logging::format_exception(e));
            }

            server_state = INITIALIZED;
        }

        /**
         * @brief runs the server I/O loop (blocking)
         * 
         * @throws serving_error if error occurred while running the server
         * @throws launch_error if couldn't run because of invalid state
         */
        void run()
        {
            if (server_state == UNINITIALIZED)
                throw launch_error("called server::run() before server::initialize()");
            else if (server_state != INITIALIZED)
                throw launch_error("called server::run() multiple times (msglink server instance is single use, cannot run multiple times)");
            
            try
            {
                // listening happens on port defined in settings
                m_socket_server.listen(m_port);

                // start accepting
                m_socket_server.start_accept();
            }
            catch(const std::exception& e)
            {
                throw launch_error(el::logging::format_exception(e));
            }

            server_state = RUNNING;

            try
            {
                // run the io loop
                m_socket_server.run();
            }
            catch(const std::exception& e)
            {
                server_state = FAILED;
                throw serving_error(el::logging::format_exception(e));
            }

            server_state = STOPPED;
        }

        /**
         * @brief stops the server if it is running and does nothing
         * otherwise.
         * 
         * @throws termination_error if stopping was attempted but failed
         */
        void stop()
        {
            // do nothing if server is not running
            if (server_state != RUNNING) return;

            try
            {
                // stop listening for new connections
                m_socket_server.stop_listening();

                // close all existing connections
                for (const auto& [hdl, client] : open_connections)
                {
                    m_socket_server.close(hdl, 0, "server stopped");
                }
            }
            catch(const std::exception& e)
            {
                throw termination_error(el::logging::format_exception(e));
            }
        }

    };

} // namespace el