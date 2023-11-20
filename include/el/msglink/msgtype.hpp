/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
20.11.23, 18:35
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Defines all message types possible and conversions from/to string
*/

#pragma once

#include <string>

#include "errors.hpp"


namespace el::msglink
{
    enum class msg_type_t
    {
        LOGIN,
        LOGIN_ACK,
        EVENT_SUB,
        EVENT_SUB_ACK,
        EVENT_SUB_NAK,
        EVENT_UNSUB,
        EVENT_EMIT,
        DATA_SUB,
        DATA_SUB_ACK,
        DATA_SUB_NAK,
        DATA_UNSUB,
        DATA_CHANGE,
        RPC_CALL,
        RPC_NAK,
        RPC_ERR,
        RPC_RESULT,
    };

    const char *msg_type_to_string(const msg_type_t _msg_type) noexcept
    {
        switch (_msg_type)
        {
            using enum msg_type_t;

        case LOGIN:
            return "login";
            break;
        case LOGIN_ACK:
            return "login_ack";
            break;
        case EVENT_SUB:
            return "event_sub";
            break;
        case EVENT_SUB_ACK:
            return "event_sub_ack";
            break;
        case EVENT_SUB_NAK:
            return "event_sub_nak";
            break;
        case EVENT_UNSUB:
            return "event_unsub";
            break;
        case EVENT_EMIT:
            return "event_emit";
            break;
        case DATA_SUB:
            return "data_sub";
            break;
        case DATA_SUB_ACK:
            return "data_sub_ack";
            break;
        case DATA_SUB_NAK:
            return "data_sub_nak";
            break;
        case DATA_UNSUB:
            return "data_unsub";
            break;
        case DATA_CHANGE:
            return "data_change";
            break;
        case RPC_CALL:
            return "rpc_call";
            break;
        case RPC_NAK:
            return "rpc_nak";
            break;
        case RPC_ERR:
            return "rpc_err";
            break;
        case RPC_RESULT:
            return "rpc_result";
            break;
        
        default:
            throw invalid_msg_type_error("Invalid enum value: " + std::to_string((int)_msg_type));
        }
    }

    msg_type_t msg_type_from_string(const std::string &_msg_type_name) noexcept
    {
        using enum msg_type_t;

        if (_msg_type_name ==  "login")
            return LOGIN;
        else if (_msg_type_name ==  "login_ack")
            return LOGIN_ACK;
        else if (_msg_type_name ==  "event_sub")
            return EVENT_SUB;
        else if (_msg_type_name ==  "event_sub_ack")
            return EVENT_SUB_ACK;
        else if (_msg_type_name ==  "event_sub_nak")
            return EVENT_SUB_NAK;
        else if (_msg_type_name ==  "event_unsub")
            return EVENT_UNSUB;
        else if (_msg_type_name ==  "event_emit")
            return EVENT_EMIT;
        else if (_msg_type_name ==  "data_sub")
            return DATA_SUB;
        else if (_msg_type_name ==  "data_sub_ack")
            return DATA_SUB_ACK;
        else if (_msg_type_name ==  "data_sub_nak")
            return DATA_SUB_NAK;
        else if (_msg_type_name ==  "data_unsub")
            return DATA_UNSUB;
        else if (_msg_type_name ==  "data_change")
            return DATA_CHANGE;
        else if (_msg_type_name ==  "rpc_call")
            return RPC_CALL;
        else if (_msg_type_name ==  "rpc_nak")
            return RPC_NAK;
        else if (_msg_type_name ==  "rpc_err")
            return RPC_ERR;
        else if (_msg_type_name ==  "rpc_result")
            return RPC_RESULT;
        else
            throw invalid_msg_type_error("Invalid type name: " + _msg_type_name);
    }
} // namespace el::msglink

