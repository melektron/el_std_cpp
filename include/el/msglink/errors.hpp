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
#include <asio/error.hpp>

#include "../exceptions.hpp"
#include "internal/ws_close_code.hpp"


namespace el::msglink
{
    namespace wspp = websocketpp;

    /**
     * @brief base class for all errors related to msglink
     */
    class msglink_error : public el::exception
    {
        using el::exception::exception;
    };

    /**
     * @brief exception indicating that initialization
     * could not be performed for some reason
     */
    class initialization_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };
    
    /**
     * @brief exception indicating that the server/client couldn't be
     * started because of incorrect state (e.g. not initialized)
     */
    class launch_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief tried to access invalid connection instance
     * (wspp callback with unknown connection handle)
     */
    class invalid_connection_error: public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief msglink error to represent wspp error.
     * (which is basically just asio error)
     * All wspp::exceptions will be caught and rethrown as
     * socket_errors by msglink, so everything inherits from
     * msglink_error 
     */
    class socket_error : public msglink_error
    {
    public:

        wspp::lib::error_code m_code;   // is just asio::error_code which should be std::error_code on modern system

        socket_error(const wspp::exception &_e)
            : msglink_error(_e.m_msg)
            , m_code(_e.m_code)
        {}

        wspp::lib::error_code code() const noexcept {
            return m_code;
        }

    };

    /**
     * @brief received malformed event which either couldn't be parsed
     * as json or was otherwise structurally invalid, e.g. missing properties.
     */
    class malformed_message_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief attempted to create or register a new transaction but a 
     * transaction with the same ID already exists.
     */
    class duplicate_transaction_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief attempted to retrieve an active transaction with invalid ID
     * or the active transaction does not match the required type.
     */
    class invalid_transaction_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief received messages which do not conform to the expected
     * conversation as defined by the protocol.
     */
    class protocol_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief attempted to access some sort of object like
     * an event subscription for subscribing/unsubscribing
     * but the identifier (name, id number, ...) is invalid
     * and the object cannot be accessed.
     */
    class invalid_identifier_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief link is not compatible with the link of the other party.
     * This may be thrown during authentication.
     */
    class incompatible_link_error : public msglink_error
    {
        close_code_t m_code;

    public:

        incompatible_link_error(close_code_t _code ,const char *_msg)
            : msglink_error(_msg)
            , m_code(_code)
        {}
        
        template<typename... _Args>
        incompatible_link_error(close_code_t _code, const std::string &_msg_fmt, _Args... _args)
            : msglink_error(_msg_fmt, _args...)
            , m_code(_code)
        {}

        close_code_t code() const noexcept
        {
            return m_code;
        }
    };

    /**
     * @brief received unknown (invalid) incoming msglink event.
     * This means, the event is either not defined or defined as
     * outgoing only. This is a protocol error because undefined
     * events should be detected and caught during authentication.
     * Transmitting messages for unknown events after auth success
     * does not conform to the protocol and is likely the result 
     * of a library implementation mistake.
     */
    class invalid_incoming_event_error : public protocol_error
    {
        using protocol_error::protocol_error;
    };

    /**
     * @brief attempted to emit an unknown (invalid) outgoing msglink event.
     * This means, the event is either not defined or defined as
     * incoming only.
     * This is only thrown as a result of local emit function calls.
     */
    class invalid_outgoing_event_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief this is thrown in a remote function call when the remote
     * party responds with an error. 
     * The what() string contains the info provided by the remote client
     */
    class remote_function_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

    /**
     * @brief exception used to indicate an unexpected
     * error code occurred like e.g. in some asio-related operation.
     * This uses std::error_code which is the same as
     * asio::error_code and therefor also wspp::error_code in modern C++.
     * Multiple error code enums might be used though.
     */
    class unexpected_error : public msglink_error
    {
    public:

        std::error_code m_code;    // should be just std::error_code on modern systems 

        unexpected_error(const std::error_code &_ec)
            : msglink_error(_ec.message())
            , m_code(_ec)
        {}

        std::error_code code() const noexcept {
            return m_code;
        }
    };

    /**
     * @brief tried to parse/serialize an invalid message type.
     */
    class invalid_msg_type_error : public msglink_error
    {
        using msglink_error::msglink_error;
    };

} // namespace el::msglink
