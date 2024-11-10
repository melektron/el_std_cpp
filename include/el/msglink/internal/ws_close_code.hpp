/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
29.11.23, 09:38
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Websocket close codes specific to msglink.
*/

#pragma once

namespace el::msglink
{
    enum class close_code_t : uint16_t
    {
        CLOSED_BY_USER = 1000,
        PROTO_VERSION_INCOMPATIBLE = 3001,
        LINK_VERSION_MISMATCH = 3002,
        EVENT_REQUIREMENTS_NOT_SATISFIED = 3003,
        DATA_SOURCE_REQUIREMENTS_NOT_SATISFIED = 3004,
        FUNCTION_REQUIREMENTS_NOT_SATISFIED = 3005,
        MALFORMED_MESSAGE = 3006,
        PROTOCOL_ERROR = 3007,
        UNDEFINED_LINK_ERROR = 3100
    };

    inline const char *close_code_name(close_code_t _code)
    {
        switch (_code)
        {
            using enum close_code_t;

            case CLOSED_BY_USER:
                return "closed by user";
            case PROTO_VERSION_INCOMPATIBLE:
                return "proto version incompatible";
            case LINK_VERSION_MISMATCH:
                return "link version mismatch";
            case EVENT_REQUIREMENTS_NOT_SATISFIED:
                return "event requirements not satisfied";
            case DATA_SOURCE_REQUIREMENTS_NOT_SATISFIED:
                return "data source requirements not satisfied";
            case FUNCTION_REQUIREMENTS_NOT_SATISFIED:
                return "function requirements not satisfied";
            case MALFORMED_MESSAGE:
                return "malformed message";
            case PROTOCOL_ERROR:
                return "protocol error";
            case UNDEFINED_LINK_ERROR:
                return "undefined link error";
            default:
                return "N/A";
        };
    }
} // namespace el::msglink
