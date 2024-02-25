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
#include "internal/context.hpp"
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
     * the msglink class tree, they are not end-user facing. This class expects
     * the global class tree guard to be locked for all methods called from outside
     * (there are also some asio callbacks inside)
     */
    template<std::derived_from<link> _LT>
    class connection_handler : public link_interface
    {

    private:    // state
        // global class tree context passed from server
        ct_context &ctx;

        // the websocket server managing this client connection (passed from msglink server)
        wsserver &socket_server;

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
            return socket_server.get_con_from_hdl(m_connection);
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
         * 
         * @attention (external entry: asio cb)
         */
        void handle_ping_timer(const std::error_code &_ec)
        {
            auto lock = ctx.get_lock();

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
        
        /**
         * @brief closes the connection with one of the custom
         * msglink close codes and logs the event to console
         * 
         * @param _code 
         */
        void close_with_log(close_code_t _code)
        {
            EL_LOGI(
                "Closing connection with code %d (%s)", 
                _code,
                close_code_name(_code)
            );
            get_connection()->close(
                (uint16_t)_code, 
                close_code_name(_code)
            );
        }

        /**
         * @brief runs a passed lambda containing a link method call 
         * and handles exceptions thrown by the link, closing the connection
         * when that happens.
         * When a link throws an incompatible_link_error, the connection is
         * closed with the appropriate close code. 
         * Other exceptions cause the connection to close code for
         * undefined error.
         * 
         * @param _lambda code to run with exception handling
         */
        void run_with_exception_handling(std::function<void()> _lambda)
        {
            try
            {
                _lambda();
            }
            catch (const incompatible_link_error &e)
            {
                EL_LOG_EXCEPTION_MSG("Remote link is not compatible", e);
                close_with_log(e.code());
            }
            catch (const invalid_transaction_error &e)
            {
                EL_LOG_EXCEPTION_MSG("Invalid transaction", e);
                EL_LOGW("Ignoring invalid transaction message");
            }
            catch (const malformed_message_error &e)
            {
                EL_LOG_EXCEPTION_MSG("Received malformed data", e);
                close_with_log(close_code_t::MALFORMED_MESSAGE);
            }
            catch (const protocol_error &e)
            {
                EL_LOG_EXCEPTION_MSG("Communication doesn not comply with protocol", e);
                close_with_log(close_code_t::PROTOCOL_ERROR);
            }
            catch (const std::exception &e)
            {
                EL_LOG_EXCEPTION_MSG("Unknown exception in link", e);
                close_with_log(close_code_t::UNDEFINED_LINK_ERROR);
            }
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
         * @param _ctx global class tree context
         * @param _socket_server websocket server the connection belongs to
         * @param _connection handle to the connection
         */
        connection_handler(ct_context &_ctx, wsserver &_socket_server, wspp::connection_hdl _connection)
            : ctx(_ctx)
            , socket_server(_socket_server)
            , m_connection(_connection)
            , m_link(
                ctx,
                true,   // is server instance
                *this   // use this connection handler to communicate
            )
        {
            EL_LOG_FUNCTION_CALL();

            // define the link protocol
            run_with_exception_handling([&](){
                this->m_link.define();
            });
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
            run_with_exception_handling([&](){
                this->m_link.on_connection_established();
            });
        }

        /**
         * @brief called by server when message arrives for this connection
         * 
         * @param _msg the message to handle
         */
        void on_message(wsserver::message_ptr _msg)
        {
            EL_LOGD("Incoming Message: %s", _msg->get_payload().c_str());
            run_with_exception_handling([&](){
                this->m_link.on_message(_msg->get_payload());
            });
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
            run_with_exception_handling([&](){
                m_link.on_pong_received();
            });

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
        // global communication class tree context
        ct_context ctx;

        // == Configuration 
        // port to serve on
        int port;

        // == State
        // the websocket server used for transport
        wsserver socket_server;

        // enumeration managing current server state
        enum server_state_t
        {
            UNINITIALIZED = 0,  // newly instantiated, not initialized
            INITIALIZED = 1,    // initialize() successful
            RUNNING = 2,        // run() called, server still running
            FAILED = 3,         // run() exited with error
            STOPPED = 4         // run() exited cleanly (through stop() or other natural way)
        };
        std::atomic<server_state_t> server_state { UNINITIALIZED };

        // set of connections to corresponding connection handler instance
        std::map<
            wspp::connection_hdl,
            connection_handler<_LT>,
            std::owner_less<wspp::connection_hdl>
        > open_connections;

    private:

        /**
         * @brief new websocket connection opened (fully connected)
         * This instantiates a connection handler.
         * 
         * @attention (external entry: asio cb)
         * @param hdl websocket connection handle
         */
        void on_open(wspp::connection_hdl _hdl)
        {
            auto lock = ctx.get_lock();

            if (server_state != RUNNING)
                return;

            // create new handler instance and save it
            auto new_connection = open_connections.emplace(
                std::piecewise_construct,   // Needed for in-place construct https://en.cppreference.com/w/cpp/utility/piecewise_construct
                std::forward_as_tuple(_hdl),
                std::forward_as_tuple(ctx, socket_server, _hdl)
            );

            // notify new connection handler to start communication
            new_connection.first->second.on_open();
        }
        
        /**
         * @brief message received from a connection.
         * This forwards the call to the appropriate connection handler
         * or throws if the connection is invalid.
         * 
         * @attention (external entry: asio cb)
         * @param _hdl ws connection handle
         * @param _msg message that was received
         */
        void on_message(wspp::connection_hdl _hdl, wsserver::message_ptr _msg)
        {
            auto lock = ctx.get_lock();

            if (server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                open_connections.at(_hdl).on_message(_msg);
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
         * @attention (external entry: asio cb)
         * @param _hdl ws connection handle that has been closed
         */
        void on_close(wspp::connection_hdl _hdl)
        {
            auto lock = ctx.get_lock();

            if (server_state != RUNNING)
                return;

            if (!open_connections.contains(_hdl))
            {
                throw invalid_connection_error("Attempted to close an unknown/invalid connection which doesn't seem to exist.");
            }

            // notify connection to stop communication
            open_connections.at(_hdl).on_close();

            // remove closed connection from connection map, deleting the connection handlers
            open_connections.erase(_hdl);
        }

        /**
         * @brief called by wspp when a new connection was attempted but failed
         * before it was fully connected.
         * 
         * @attention (external entry: asio cb)
         * @param _hdl handle to associated ws connection 
         */
        void on_fail(wspp::connection_hdl _hdl)
        {
            //auto lock = ctx.get_lock();
            EL_LOG_FUNCTION_CALL();
        }

        /**
         * @brief called by wspp when a pong is received.
         * This is forwarded to the connection handler.
         * 
         * @attention (external entry: asio cb)
         * @param _hdl handle to associated ws connection 
         */
        void on_pong_received(wspp::connection_hdl _hdl, std::string _payload)
        {
            auto lock = ctx.get_lock();

            if (server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                open_connections.at(_hdl).on_pong_received(_payload);
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
         * @attention (external entry: asio cb)
         * @param _hdl handle to connection where timeout occurred
         */
        void on_pong_timeout(wspp::connection_hdl _hdl, std::string _expected_payload)
        {
            auto lock = ctx.get_lock();

            if (server_state != RUNNING)
                return;

            // forward message to connection handler
            try
            {
                open_connections.at(_hdl).on_pong_timeout(_expected_payload);
            }
            catch (const std::out_of_range &e)
            {
                throw invalid_connection_error("Pong timeout on unknown/invalid connection.");
            }
        }

    public:

        /**
         * @brief Construct a new server object
         * 
         * @attention (external entry: public method)
         * @param _port TCP port to listen on
         */
        server(int _port)
            : port(_port)
        {
            EL_LOG_FUNCTION_CALL();
        }

        // never copy or move a server
        server(const server &) = delete;
        server(server &&) = delete;

        /**
         * @attention (external entry: public method)
         */
        ~server()
        {
            EL_LOG_FUNCTION_CALL();
        }

        /**
         * @brief initializes the server setting up all transport settings
         * and preparing the server to run. This MUST be called before run().
         *
         * @attention (external entry: public method)
         * @throws msglink::initialization_error invalid state to initialize
         * @throws msglink::socket_error error while configuring networking
         * @throws other std exceptions possible
         */
        void initialize()
        {
            auto lock = ctx.get_lock();

            if (server_state != UNINITIALIZED)
                throw initialization_error("msglink server instance is single use, cannot re-initialize");

            try
            {
                // wspp log messages off by default
                socket_server.clear_access_channels(wspp::log::alevel::all);
                socket_server.clear_error_channels(wspp::log::elevel::all);
                // turn on selected logging channels
                socket_server.set_access_channels(wspp::log::alevel::disconnect);
                socket_server.set_access_channels(wspp::log::alevel::connect);
                socket_server.set_access_channels(wspp::log::alevel::fail);
                socket_server.set_error_channels(wspp::log::elevel::all);
                //socket_server.set_access_channels(wspp::log::alevel::all);

                // initialize asio communication
                socket_server.init_asio();

                // register callback handlers (More handlers: https://docs.websocketpp.org/reference_8handlers.html)
                socket_server.set_open_handler(std::bind(&server::on_open, this, pl::_1));
                socket_server.set_message_handler(std::bind(&server::on_message, this, pl::_1, pl::_2));
                socket_server.set_close_handler(std::bind(&server::on_close, this, pl::_1));
                socket_server.set_fail_handler(std::bind(&server::on_fail, this, pl::_1));
                socket_server.set_pong_handler(std::bind(&server::on_pong_received, this, pl::_1, pl::_2));
                socket_server.set_pong_timeout_handler(std::bind(&server::on_pong_timeout, this, pl::_1, pl::_2));

                // set reuse addr flag to allow faster restart times
                socket_server.set_reuse_addr(true);

            }
            catch (const wspp::exception &e)
            {
                throw socket_error(e);
            }

            server_state = INITIALIZED;
        }

        /**
         * @brief runs the server I/O loop (blocking)
         *
         * @attention (external entry: public method)
         * @throws msglink::launch_error couldn't run server because of invalid state (e.g. not initialized)
         * @throws msglink::socket_error network communication / websocket error occurred
         * @throws other msglink::msglink_error?
         * @throws other std exceptions possible
         */
        void run()
        {
            auto lock = ctx.get_lock();
            
            if (server_state == UNINITIALIZED)
                throw launch_error("called server::run() before server::initialize()");
            else if (server_state != INITIALIZED)
                throw launch_error("called server::run() multiple times (msglink server instance is single use, cannot run multiple times)");
            
            try
            {
                // listen on configured port
                socket_server.listen(port);

                // start accepting
                socket_server.start_accept();
                
                server_state = RUNNING;
                // free the lock so state can be accessed by callbacks
                lock.unlock();
                // run the io loop
                socket_server.run();
                // re-acquire ownership after loop exit. This might block until close function is done
                lock.lock();
                server_state = STOPPED;
            }
            catch (const wspp::exception &e)
            {
                server_state = FAILED;
                throw socket_error(e);
            }
            catch (...)
            {
                server_state = FAILED;
                throw;
            }

        }

        /**
         * @brief stops the server if it is running, does nothing
         * otherwise (if it's not running).
         * 
         * This can be called from any thread.
         *
         * @attention (external entry: public method)
         * @throws msglink::socket_error networking error occurred while stopping server
         * @throws other msglink::msglink_error?
         */
        void stop()
        {
            auto lock = ctx.get_lock();

            // do nothing if server is not running
            if (server_state != RUNNING) return;

            try
            {
                // stop listening for new connections
                socket_server.stop_listening();

                // close all existing connections
                for (auto &[hdl, client] : open_connections)
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

            // at this point mutex will be unlocked and run function will finish
        }

    };

} // namespace el