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
#include "../../codable.hpp"
#include "types.hpp"

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

    struct msg_auth_t
        : public base_msg_t
        , public codable
    {
        proto_version_t proto_version;
        link_version_t link_version;
        std::vector<std::string> events;
        std::vector<std::string> data_sources;
        std::vector<std::string> procedures;

        EL_DEFINE_CODABLE(
            msg_auth_t,
            type,
            tid,
            proto_version,
            link_version,
            events,
            data_sources,
            procedures
        )
    };

    struct msg_auth_ack_t
        : public base_msg_t
        , public codable
    {
        EL_DEFINE_CODABLE(
            msg_auth_ack_t,
            type,
            tid
        )
    };
} // namespace el
