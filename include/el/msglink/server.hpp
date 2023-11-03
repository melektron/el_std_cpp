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
#include <mutex>
#include <condition_variable>
#include <map>
#include <functional>
#include <chrono>
#include <csignal>

#include <el/retcode.hpp>
#include <el/logging.hpp>

#include <el/msglink/wspp.hpp>
#include <el/msglink/errors.hpp>


#define PRINT_CALL std::cout << __PRETTY_FUNCTION__ << std::endl


namespace el::msglink
{
    using namespace std::chrono_literals;

    class server;
    class connection_handler;

    class connection_handler
    {
        friend class server;

    private:    // state

        // the server managing this client connection
        wsserver &m_socket_server;

        // a handle to the connection handled by this client
        wspp::connection_hdl m_connection;


    public:

        connection_handler(wsserver &_socket_server, wspp::connection_hdl _connection)
            : m_socket_server(_socket_server)
            , m_connection(_connection)
        {
            PRINT_CALL;
        }

        // connection handler is supposed to be instantiated in-place exactly once per 
        // connection in the connection map. It should never be moved or copied.
        connection_handler(const connection_handler &) = delete;
        connection_handler(connection_handler &&) = delete;

        virtual ~connection_handler()
        {
            PRINT_CALL;
        }

        /**
         * @brief called by server when message arrives for this connection
         * 
         * @param _msg the message to handle
         */
        void on_message(wsserver::message_ptr _msg) noexcept
        {
            std::cout << "message: " << _msg->get_payload() << std::endl;
            m_socket_server.send(m_connection, _msg->get_payload(), _msg->get_opcode());
        }

        /**
         * @brief initiates a websocket ping.
         * This is called periodically by server thread.
         */
        void initiate_ping()
        {
            m_socket_server.ping(m_connection, ""); // no message needed
        }
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
        std::atomic<server_state_t> m_server_state { UNINITIALIZED };

        // set of connections to corresponding connection handler instance
        std::map<
            wspp::connection_hdl,
            connection_handler,
            std::owner_less<wspp::connection_hdl>
        > m_open_connections;

        // connection processing thread. This thread is responsible for
        // some housekeeping work such as keepalive for all the connections.
        std::thread m_processing_thread;

        // mutex and condition variable guarded flag to tell the thread
        // when to exit
        bool m_threxit;
        std::mutex m_threxit_mutex;
        std::condition_variable m_threxit_cv;

    private:

        /**
         * @brief new websocket connection opened (fully connected)
         *
         * @param hdl websocket connection handle
         */
        void on_open(wspp::connection_hdl _hdl)
        {
            PRINT_CALL;

            if (m_server_state != RUNNING)
                return;

            // create new handler instance and save it
            m_open_connections.emplace(
                std::piecewise_construct,   // Needed for in-place construct https://en.cppreference.com/w/cpp/utility/piecewise_construct
                std::forward_as_tuple(_hdl),
                std::forward_as_tuple(m_socket_server, _hdl)
            );

        }
        void on_message(wspp::connection_hdl _hdl, wsserver::message_ptr _msg)
        {
            PRINT_CALL;

            if (m_server_state != RUNNING)
                return;

            // forward message to client handler
            try
            {
                m_open_connections.at(_hdl).on_message(_msg);
            }
            catch (const std::out_of_range &e)
            {
                throw invalid_connection_error("Received message from unknown/invalid connection.");
            }

        }
        void on_close(wspp::connection_hdl _hdl)
        {
            PRINT_CALL;

            if (m_server_state != RUNNING)
                return;

            // remove closed connection from connection map
            if (!m_open_connections.erase(_hdl))
            {
                throw invalid_connection_error("Attempted to close an unknown/invalid connection which doesn't seem to exist.");
            }
        }
        
        /**
         * @brief called by wspp when a pong message times out. This is 
         * used by the keepalive system to detect connection loss
         * 
         * @param _hdl handle to connection where timeout occurred
         */
        void on_pong_timeout(wspp::connection_hdl _hdl)
        {
            PRINT_CALL;

            if (m_server_state != RUNNING)
                return;

            // if we already timed out, terminate the connection with no handshake
            // (this will still call close handler)
            wsserver::connection_ptr con = m_socket_server.get_con_from_hdl(_hdl);
            con->terminate(std::make_error_code(std::errc::timed_out));
        }

        /**
         * @brief function of the connection processing thread
         */
        void processing_thread_fn() noexcept
        {
            try
            {
                for (;;)
                {
                    std::unique_lock lock(m_threxit_mutex);
                    m_threxit_cv.wait_for(lock, 10s);
                    // mutex is now locked regardless of timeout or not
                    if (m_threxit)
                        break;

                    // if server is not running there is nothing to do
                    if (m_server_state != RUNNING)
                        continue;

                    // perform routine task
                    std::cout << "routine" << std::endl;

                    for (auto &[connection_handle, handler] : m_open_connections)
                    {
                        handler.initiate_ping();
                    }

                }

                std::cout << "thread exit" << std::endl;
            }
            catch (const std::exception &e)
            {
                // TODO: park exception here and re-raise it in run() for user handling
                std::cout << "Exception in server thread: " << el::logging::format_exception(e) << std::endl;
            }
            
        }

    public:

        server(int _port)
            : m_port(_port)
        {
            PRINT_CALL;

            // start processing thread
            m_processing_thread = std::thread(std::bind(&server::processing_thread_fn, this));
        }

        // never copy or move a server
        server(const server &) = delete;
        server(server &&) = delete;

        ~server()
        {
            PRINT_CALL;

            // order processing thread to exit
            {
                std::lock_guard lock(m_threxit_mutex);
                m_threxit = true;
            }
            m_threxit_cv.notify_one();
            // wait for thread to exit
            if (m_processing_thread.joinable())
                m_processing_thread.join();

        }

        /**
         * @brief initializes the server setting up all transport settings
         * and preparing the server to run. This MUST be called before run().
         *
         * @throws msglink::initialization_error invalid state to initialize
         * @throws msglink::socket_error error while configuring networking
         * @throws other std exceptions possible
         */
        void initialize()
        {
            if (m_server_state != UNINITIALIZED)
                throw initialization_error("msglink server instance is single use, cannot re-initialize");

            try
            {
                // we don't want any wspp log messages
                m_socket_server.set_access_channels(wspp::log::alevel::all);
                m_socket_server.set_error_channels(wspp::log::elevel::all);

                // initialize asio communication
                m_socket_server.init_asio();

                // register callback handlers (More handlers: https://docs.websocketpp.org/reference_8handlers.html)
                m_socket_server.set_open_handler(std::bind(&server::on_open, this, pl::_1));
                m_socket_server.set_message_handler(std::bind(&server::on_message, this, pl::_1, pl::_2));
                m_socket_server.set_close_handler(std::bind(&server::on_close, this, pl::_1));
                m_socket_server.set_pong_timeout_handler(std::bind(&server::on_pong_timeout, this, pl::_1));

                // set reuse addr flag to allow faster restart times
                m_socket_server.set_reuse_addr(true);

            }
            catch (const wspp::exception &e)
            {
                throw socket_error(e);
            }

            m_server_state = INITIALIZED;
        }

        /**
         * @brief runs the server I/O loop (blocking)
         *
         * @throws msglink::launch_error couldn't run server because of invalid state (e.g. not initialized)
         * @throws msglink::socket_error network communication / websocket error occurred
         * @throws other msglink::msglink_error?
         * @throws other std exceptions possible
         */
        void run()
        {
            if (m_server_state == UNINITIALIZED)
                throw launch_error("called server::run() before server::initialize()");
            else if (m_server_state != INITIALIZED)
                throw launch_error("called server::run() multiple times (msglink server instance is single use, cannot run multiple times)");

            try
            {
                // listening happens on port defined in settings
                m_socket_server.listen(m_port);

                // start accepting
                m_socket_server.start_accept();

                // run the io loop
                m_server_state = RUNNING;
                m_socket_server.run();
                m_server_state = STOPPED;
            }
            catch (const wspp::exception &e)
            {
                m_server_state = FAILED;
                throw socket_error(e);
            }
            catch (...)
            {
                m_server_state = FAILED;
                throw;
            }

        }

        /**
         * @brief stops the server if it is running and does nothing
         * otherwise.
         * 
         * This can be called from any thread, wspp is thread safe.
         *
         * @throws msglink::socket_error networking error occurred while stopping server
         * @throws other msglink::msglink_error?
         */
        void stop()
        {
            // do nothing if server is not running
            if (m_server_state != RUNNING) return;

            try
            {
                // stop listening for new connections
                m_socket_server.stop_listening();

                // close all existing connections
                for (const auto &[hdl, client] : m_open_connections)
                {
                    m_socket_server.close(hdl, 0, "server stopped");
                }
            }
            catch (const wspp::exception &e)
            {
                throw socket_error(e);
            }

        }

    };

} // namespace el