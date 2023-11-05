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
#include <el/exceptions.hpp>

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

} // namespace el::msglink