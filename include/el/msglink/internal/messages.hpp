/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
26.11.23, 20:30
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Codables for internal communication messages
*/

#pragma once

#include <cstdint>
#include <set>

#include "../../codable.hpp"
#include "types.hpp"
#include "msgtype.hpp"

namespace el::msglink
{
    /**
     * @brief base type for all messages
     */
    struct base_msg_t
    {
        std::string type;
        tid_t tid;
    };

    struct msg_pong_t
        // not base message because no tid required
        : public encodable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_PONG;

        EL_DEFINE_ENCODABLE(
            msg_evt_sub_t,
            type
        )
    };

    struct msg_auth_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_AUTH;
        proto_version_t proto_version;
        link_version_t link_version;
        std::optional<bool> no_ping;
        std::set<std::string> events;
        std::set<std::string> data_sources;
        std::set<std::string> functions;

        EL_DEFINE_CODABLE(
            msg_auth_t,
            type,
            tid,
            proto_version,
            link_version,
            no_ping,
            events,
            data_sources,
            functions
        )
    };

    struct msg_auth_ack_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_AUTH_ACK;
        EL_DEFINE_CODABLE(
            msg_auth_ack_t,
            type,
            tid
        )
    };

    struct msg_evt_sub_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_EVT_SUB;
        std::string name;

        EL_DEFINE_CODABLE(
            msg_evt_sub_t,
            type,
            tid,
            name
        )
    };

    struct msg_evt_unsub_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_EVT_UNSUB;
        std::string name;

        EL_DEFINE_CODABLE(
            msg_evt_sub_t,
            type,
            tid,
            name
        )
    };

    struct msg_evt_emit_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_EVT_EMIT;
        std::string name;
        nlohmann::json data;

        EL_DEFINE_CODABLE(
            msg_evt_emit_t,
            type,
            tid,
            name,
            data
        )
    };

    struct msg_func_call_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_FUNC_CALL;
        std::string name;
        nlohmann::json params;

        EL_DEFINE_CODABLE(
            msg_func_call_t,
            type,
            tid,
            name,
            params
        )
    };

    struct msg_func_err_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_FUNC_ERR;
        std::string info;

        EL_DEFINE_CODABLE(
            msg_func_call_t,
            type,
            tid,
            info
        )
    };

    struct msg_func_result_t
        : public base_msg_t
        , public codable
    {
        std::string type = __EL_MSGLINK_MSG_NAME_FUNC_RESULT;
        nlohmann::json results;

        EL_DEFINE_CODABLE(
            msg_func_call_t,
            type,
            tid,
            results
        )
    };

} // namespace el
