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

#include "../errors.hpp"


#define __EL_MSGLINK_MSG_NAME_AUTH          "auth"
#define __EL_MSGLINK_MSG_NAME_AUTH_ACK      "auth_ack"
#define __EL_MSGLINK_MSG_NAME_EVT_SUB       "evt_sub"
#define __EL_MSGLINK_MSG_NAME_EVT_SUB_ACK   "evt_sub_ack"
#define __EL_MSGLINK_MSG_NAME_EVT_SUB_NAK   "evt_sub_nak"
#define __EL_MSGLINK_MSG_NAME_EVT_UNSUB     "evt_unsub"
#define __EL_MSGLINK_MSG_NAME_EVT_EMIT      "evt_emit"
#define __EL_MSGLINK_MSG_NAME_DATA_SUB      "data_sub"
#define __EL_MSGLINK_MSG_NAME_DATA_SUB_ACK  "data_sub_ack"
#define __EL_MSGLINK_MSG_NAME_DATA_SUB_NAK  "data_sub_nak"
#define __EL_MSGLINK_MSG_NAME_DATA_UNSUB    "data_unsub"
#define __EL_MSGLINK_MSG_NAME_DATA_CHANGE   "data_change"
#define __EL_MSGLINK_MSG_NAME_RPC_CALL      "rpc_call"
#define __EL_MSGLINK_MSG_NAME_RPC_NAK       "rpc_nak"
#define __EL_MSGLINK_MSG_NAME_RPC_ERR       "rpc_err"
#define __EL_MSGLINK_MSG_NAME_RPC_RESULT    "rpc_result"


namespace el::msglink
{
    enum class msg_type_t
    {
        AUTH,
        AUTH_ACK,
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

    inline const char *msg_type_to_string(const msg_type_t _msg_type)
    {
        switch (_msg_type)
        {
            using enum msg_type_t;

        case AUTH:
            return __EL_MSGLINK_MSG_NAME_AUTH;
            break;
        case AUTH_ACK:
            return __EL_MSGLINK_MSG_NAME_AUTH_ACK;
            break;
        case EVENT_SUB:
            return __EL_MSGLINK_MSG_NAME_EVT_SUB;
            break;
        case EVENT_SUB_ACK:
            return __EL_MSGLINK_MSG_NAME_EVT_SUB_ACK;
            break;
        case EVENT_SUB_NAK:
            return __EL_MSGLINK_MSG_NAME_EVT_SUB_NAK;
            break;
        case EVENT_UNSUB:
            return __EL_MSGLINK_MSG_NAME_EVT_UNSUB;
            break;
        case EVENT_EMIT:
            return __EL_MSGLINK_MSG_NAME_EVT_EMIT;
            break;
        case DATA_SUB:
            return __EL_MSGLINK_MSG_NAME_DATA_SUB;
            break;
        case DATA_SUB_ACK:
            return __EL_MSGLINK_MSG_NAME_DATA_SUB_ACK;
            break;
        case DATA_SUB_NAK:
            return __EL_MSGLINK_MSG_NAME_DATA_SUB_NAK;
            break;
        case DATA_UNSUB:
            return __EL_MSGLINK_MSG_NAME_DATA_UNSUB;
            break;
        case DATA_CHANGE:
            return __EL_MSGLINK_MSG_NAME_DATA_CHANGE;
            break;
        case RPC_CALL:
            return __EL_MSGLINK_MSG_NAME_RPC_CALL;
            break;
        case RPC_NAK:
            return __EL_MSGLINK_MSG_NAME_RPC_NAK;
            break;
        case RPC_ERR:
            return __EL_MSGLINK_MSG_NAME_RPC_ERR;
            break;
        case RPC_RESULT:
            return __EL_MSGLINK_MSG_NAME_RPC_RESULT;
            break;
        
        default:
            throw invalid_msg_type_error("Invalid enum value: " + std::to_string((int)_msg_type));
        }
    }

    inline msg_type_t msg_type_from_string(const std::string &_msg_type_name)
    {
        using enum msg_type_t;

        if (_msg_type_name == __EL_MSGLINK_MSG_NAME_AUTH)
            return AUTH;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_AUTH_ACK)
            return AUTH_ACK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_EVT_SUB)
            return EVENT_SUB;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_EVT_SUB_ACK)
            return EVENT_SUB_ACK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_EVT_SUB_NAK)
            return EVENT_SUB_NAK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_EVT_UNSUB)
            return EVENT_UNSUB;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_EVT_EMIT)
            return EVENT_EMIT;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_DATA_SUB)
            return DATA_SUB;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_DATA_SUB_ACK)
            return DATA_SUB_ACK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_DATA_SUB_NAK)
            return DATA_SUB_NAK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_DATA_UNSUB)
            return DATA_UNSUB;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_DATA_CHANGE)
            return DATA_CHANGE;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_RPC_CALL)
            return RPC_CALL;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_RPC_NAK)
            return RPC_NAK;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_RPC_ERR)
            return RPC_ERR;
        else if (_msg_type_name == __EL_MSGLINK_MSG_NAME_RPC_RESULT)
            return RPC_RESULT;
        else
            throw invalid_msg_type_error("Invalid type name: " + _msg_type_name);
    }
} // namespace el::msglink

