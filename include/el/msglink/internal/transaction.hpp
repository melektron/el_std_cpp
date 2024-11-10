/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
30.11.23, 13:09
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Structures to represent running transactions
*/

#pragma once

#include <memory>
#include <functional>

#include "types.hpp"


namespace el::msglink
{
    enum class inout_t
    {
        INCOMING = 0,
        OUTGOING = 1
    };

    struct transaction_t
    {
        tid_t id;
        const inout_t direction = inout_t::OUTGOING;

        // dummy to make dynamic polymorphic type casting work
        virtual void _poly_dummy() const noexcept {};   

        bool is_incoming() const noexcept
        {
            return direction == inout_t::INCOMING;
        }

        bool is_outgoing() const noexcept
        {
            return direction == inout_t::OUTGOING;
        }

        /**
         * @brief asserts that the transaction is outgoing
         * and throws a protocol error if it is not.
         * 
         * This might be use for example when an acknowledgement message is received
         * where it doesn't make sense in some cases for the remote party to send
         * an acknowledgement to it's own request.
         * 
         * @param _exmsg message for the protocol error
         */
        void assert_is_outgoing(const char *_exmsg)
        {
            if (!is_outgoing())
                throw protocol_error(_exmsg);
        }

        transaction_t() = default;
        transaction_t(tid_t _id, inout_t _direction)
            : id(_id)
            , direction(_direction)
        {}
    };

    using transaction_ptr_t = std::shared_ptr<transaction_t>;

    struct transaction_auth_t : public transaction_t
    {
        using transaction_t::transaction_t;
    };

    struct transaction_function_call_t : public transaction_t   // only used for outgoing
    {
        // function called when the result message is received.
        std::function<void(const nlohmann::json &)> handle_result;
        // function called when the error message is received
        std::function<void(const std::string &)> handle_error;

        using transaction_t::transaction_t;
    };
    
} // namespace el::msglink
