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

#include <map>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <csignal>
#include <functional>
#include <condition_variable>
#include <concepts>

#include <asio/steady_timer.hpp>

#include "../retcode.hpp"
#include "../logging.hpp"

#include "internal/wspp.hpp"
#include "internal/link_interface.hpp"
#include "errors.hpp"
#include "link.hpp"


namespace el::msglink
{
    using namespace std::chrono_literals;

    template<std::derived_from<link> _LT>
    class server;

    template<std::derived_from<link> _LT>
    class connection_handler;

    /**
     * @brief class that is instantiated for every connection.
     * This is used inside the server class internally to perform
     * actions that are specific to but needed for all open connections.
     * 
     * Methods of this class are only allowed to be called from within
     * the main asio loop, so from the handlers in the 
     * server class.
     */
    template<std::derived_from<link> _LT>
    class connection_handler : public link_interface
    {
        friend class server<_LT>;

    private:    // state

        // the server managing this client connection
        wsserver &m_socket_server;

        // a handle to the connection handled by this client
        wspp::connection_hdl m_connection;

        // asio timer used to schedule keep-alive pings
        std::shared_ptr<asio::steady_timer> m_ping_timer;

        // link instance for handling communication with this client
        _LT m_link;

        // set when communication has been canceled to prevent any further actions
        soflag m_communication_canceled;
    
    private:    // methods

        /**
         * @brief upgrades the connection handle m_connection
         * to a full connection ptr (shared ptr to con).
         * Throws error if fails because invalid connection.
         * This should never happen and is supposed to be
         * caught in the main asio loop.
         * 
         * Should never be called from outside server io loop.
         * 
         * @return wsserver::connection_ptr the upgraded connection
         */
        wsserver::connection_ptr get_connection()
        {
            return m_socket_server.get_con_from_hdl(m_connection);
        }

        /**
         * @brief schedules a ping to be initiated
         * after the configured ping interval.
         * If a ping timer is active, it will be canceled.
         * 
         */
        void schedule_ping()
        {
            auto con = get_connection();

            // cancel existing timer if one is set
            if (m_ping_timer)
                m_ping_timer->cancel();
            
            m_ping_timer = con->set_timer(1000, std::bind(&connection_handler::handle_ping_timer, this, pl::_1));
        }

        /**
         * @brief initiates a websocket ping.
         * This is called periodically by timer.
         */
        void handle_ping_timer(const std::error_code &_ec)
        {
            // if timer was canceled, do nothing.
            if (_ec == wspp::transport::error::operation_aborted)   // the set_timer method intercepts the handler and changes the code to a non-default asio one
                return;
            else if (_ec)
                throw unexpected_error(_ec);
            
            // send a ping
            get_connection()->ping(""); // no message needed

            // when a ping is received in the pong handler, a new ping will
            // be scheduled.
        }

        /**
         * @brief called by the close_connection() and on_close() method to
         * ensure that any async communication procedures are stopped and the link is
         * disconnected before entering a potentially invalid 
         * closing-handshake state.
         * This method can be called multiple times and will do nothing after the 
         * first call.
         */
        void cancel_communication()
        {
            // if already canceled, don't cancel again
            if (m_communication_canceled)
                return;

            // TODO: invalidate the link (i.e. "disconnect" it from the link iterface
            // so it cannot call back to it anymore)

            // cancel ping timer if one is running
            if (m_ping_timer)
                m_ping_timer->cancel();

            m_communication_canceled.set();
        }
    
    protected:  // methods
        /**
         * @brief implements the link_interface interface
         * to allow the link to send messages through the client
         * communication channel.
         * 
         * @param _content message content
         */
        virtual void send_message(const std::string &_content) override
        {
            // ensure no messages go through after cancel, even though link 
            // shouldn't call this method anymore after cancel anyway.
            if (m_communication_canceled)
                return;
            
            EL_LOGD("Outgoing Message: %s", _content.c_str());
            get_connection()->send(_content);
        }

    public:

        // connection handler is supposed to be instantiated in-place exactly once per 
        // connection in the connection map. It should never be moved or copied.
        connection_handler(const connection_handler &) = delete;
        connection_handler(connection_handler &&) = delete;

        /**
         * @brief called during on_open when new connection
         * is established. Used only for initialization.
         * 
         * @param _socket_server 
         * @param _connection 
         */
        connection_handler(wsserver &_socket_server, wspp::connection_hdl _connection)
            : m_socket_server(_socket_server)
            , m_connection(_connection)
            , m_link(
                true,   // is server instance
                *this   // use this connection handler to communicate
            )
        {
            EL_LOG_FUNCTION_CALL();

            // define the link protocol
            m_link.define();
        }

        /**
         * @brief called during on_close when connection is closed or terminated.
         * Used to clean up resources like canceling any potential actions.
         */
        virtual ~connection_handler()
        {
            EL_LOG_FUNCTION_CALL();

            // cancel ping timer if one is running
            if (m_ping_timer)
                m_ping_timer->cancel();
        }

        /**
         * @brief implements the link_interface interface
         * to allow the link to close the connection at any point.
         * 
         * @param _code close status code
         * @param _reason readable reason as required by websocket protocol
         */
        virtual void close_connection(int _code, std::string _reason) noexcept override
        {
            EL_LOG_FUNCTION_CALL();

            // to avoid communication actions during closing handshake
            cancel_communication();
            // close the connection gracefully
            get_connection()->close(_code, _reason);

            // WARNING: connection_handler instance is destroyed before the terminate() call returns. 
            // Don't use it here anymore! This must also be regarded by the link.
        }


        /**
         * @brief called by server immediately after connection is
         * established (and probably this instance has been constructed).
         * This is used to initiate any communication actions.
         * 
         */
        void on_open()
        {
            EL_LOG_FUNCTION_CALL();

            // start the first ping
            schedule_ping();

            // start communication by notifying the link
            // TODO: i.e. "connecting" the link to the interface (change how this works)
            m_link.on_connection_established();
        }

        /**
         * @brief called by server when message arrives for this connection
         * 
         * @param _msg the message to handle
         */
        void on_message(wsserver::message_ptr _msg)
        {
            EL_LOGD("Incoming Message: %s", _msg->get_payload().c_str());
            m_link.on_message(_msg->get_payload());
        }

        /**
         * @brief called by server when a pong message arrives for this connection.
         * This is used to test if the connection is still alive
         * 
         * @param _payload the pong payload (not used)
         */
        void on_pong_received(std::string &_payload)
        {
            // pong arrived in time, all good, connection alive

            // notify link, so pong message can be sent to client if needed
            m_link.on_pong_received();

            // schedule a new ping to be sent a bit later.
            schedule_ping();
        }

        /**
         * @brief called by server when an expected pong message (given the sent
         * ping messages) has not arrived in time. When this happens,
         * the connection is considered dead and will be terminated.
         * 
         * @param _expected_payload the expected payload from the ping message
         */
        void on_pong_timeout(std::string &_expected_payload)
        {
            cancel_communication();

            // terminate connection
            get_connection()->terminate(std::make_error_code(std::errc::timed_out));
            
            // WARNING: connection_handler instance is destroyed before the terminate() call returns. 
            // Don't use it here anymore!
        }

        /**
         * @brief called by the server when the connection has been closed (for any reason,
         * whether initiated by client or by server) to stop any potentially still running
         * communication procedures.
         */
        void on_close()
        {
            EL_LOG_FUNCTION_CALL();

            // might already have been called by close_connection() but doesn't matter.
            cancel_communication();
        }

    };

    template<std::derived_from<link> _LT>
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
            connection_handler<_LT>,
            std::owner_less<wspp::connection_hdl>
        > m_open_connections;

    private:

        /**
         * @brief new websocket connection opened (fully connected)
         * This instantiates a connection handler.
         * @param hdl websocket connection handle
         */
        void on_open(wspp::connection_hdl _hdl)
        {
            if (m_server_state != RUNNING)
                return;

            // TODO: make atomic

            // create new handler instance and save it
            auto new_connection = m_open_connections.emplace(
                std::piecewise_construct,   // Needed for in-place construct https://en.cppreference.com/w/cpp/utility/piecewise_construct
                std::forward_as_tuple(_hdl),
                std::forward_as_tuple(m_socket_server, _hdl)
            );

            // notify new connection handler to start communication
            new_connection.first->second.on_open();
        }
        
        /**
         * @brief message received from a connection.
         * This forwards the call to the appropriate connection handler
         * or throws if the connection is invalid.
         * 
         * @param _hdl ws connection handle
         * @param _msg message that was received
         */
        void on_message(wspp::connection_hdl _hdl, wsserver::message_ptr _msg)
        {
            if (m_server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                m_open_connections.at(_hdl).on_message(_msg);
            }
            catch (const std::out_of_range &e)
            {
                throw invalid_connection_error("Received message from unknown/invalid connection.");
            }
        }
        
        /**
         * @brief websocket connection has been closed, 
         * Whether gracefully or dropped. This deletes
         * the associated connection handler and therefore
         * stops any tasks going on with that connection.
         * 
         * @param _hdl ws connection handle that has been closed
         */
        void on_close(wspp::connection_hdl _hdl)
        {
            if (m_server_state != RUNNING)
                return;

            // TODO: make thread-safe (atomic)
            if (!m_open_connections.contains(_hdl))
            {
                throw invalid_connection_error("Attempted to close an unknown/invalid connection which doesn't seem to exist.");
            }

            // notify connection to stop communication
            m_open_connections.at(_hdl).on_close();

            // remove closed connection from connection map, deleting the connection handlers
            m_open_connections.erase(_hdl);
        }

        /**
         * @brief called by wspp when a new connection was attempted but failed
         * before it was fully connected.
         * 
         * @param _hdl handle to associated ws connection 
         */
        void on_fail(wspp::connection_hdl _hdl)
        {
            EL_LOG_FUNCTION_CALL();
        }

        /**
         * @brief called by wspp when a pong is received.
         * This is forwarded to the connection handler.
         * 
         * @param _hdl handle to associated ws connection 
         */
        void on_pong_received(wspp::connection_hdl _hdl, std::string _payload)
        {
            if (m_server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                m_open_connections.at(_hdl).on_pong_received(_payload);
            }
            catch (const std::out_of_range &e)
            {
                throw invalid_connection_error("Received pong from unknown/invalid connection.");
            }
        }
        
        /**
         * @brief called by wspp when a pong message times out. This is 
         * used by the keepalive system to detect connection loss.
         * This call is forwarded to connection handler.
         * 
         * @param _hdl handle to connection where timeout occurred
         */
        void on_pong_timeout(wspp::connection_hdl _hdl, std::string _expected_payload)
        {
            if (m_server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                m_open_connections.at(_hdl).on_pong_timeout(_expected_payload);
            }
            catch (const std::out_of_range &e)
            {
                throw invalid_connection_error("Pong timeout on unknown/invalid connection.");
            }
        }

    public:

        server(int _port)
            : m_port(_port)
        {
            EL_LOG_FUNCTION_CALL();
        }

        // never copy or move a server
        server(const server &) = delete;
        server(server &&) = delete;

        ~server()
        {
            EL_LOG_FUNCTION_CALL();
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
                // wspp log messages off by default
                m_socket_server.clear_access_channels(wspp::log::alevel::all);
                m_socket_server.clear_error_channels(wspp::log::elevel::all);
                // turn on selected logging channels
                m_socket_server.set_access_channels(wspp::log::alevel::disconnect);
                m_socket_server.set_access_channels(wspp::log::alevel::connect);
                m_socket_server.set_access_channels(wspp::log::alevel::fail);
                m_socket_server.set_error_channels(wspp::log::elevel::all);
                //m_socket_server.set_access_channels(wspp::log::alevel::all);

                // initialize asio communication
                m_socket_server.init_asio();

                // register callback handlers (More handlers: https://docs.websocketpp.org/reference_8handlers.html)
                m_socket_server.set_open_handler(std::bind(&server::on_open, this, pl::_1));
                m_socket_server.set_message_handler(std::bind(&server::on_message, this, pl::_1, pl::_2));
                m_socket_server.set_close_handler(std::bind(&server::on_close, this, pl::_1));
                m_socket_server.set_fail_handler(std::bind(&server::on_fail, this, pl::_1));
                m_socket_server.set_pong_handler(std::bind(&server::on_pong_received, this, pl::_1, pl::_2));
                m_socket_server.set_pong_timeout_handler(std::bind(&server::on_pong_timeout, this, pl::_1, pl::_2));

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
                // listen on configured port
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
         * @brief stops the server if it is running, does nothing
         * otherwise (if it's not running).
         * 
         * This can be called from any thread. (TODO: make sure using mutex)
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
                for (auto &[hdl, client] : m_open_connections)
                {
                    // use close_connection method to ensure communication is properly
                    // stopped, preventing errors
                    client.close_connection(0, "server stopped");
                }
            }
            catch (const wspp::exception &e)
            {
                throw socket_error(e);
            }

        }

    };

} // namespace el