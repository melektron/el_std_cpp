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
    enum class close_code_t
    {
        CLOSED_BY_USER = 1000,
        PROTO_VERSION_INCOMPATIBLE = 3001,
        LINK_VERSION_MISMATCH = 3002,
        EVENT_REQUIREMENTS_NOT_SATISFIED = 3003,
        DATA_SOURCE_REQUIREMENTS_NOT_SATISFIED = 3004,
        RPC_REQUIREMENTS_NOT_SATISFIED = 3005,
    };
} // namespace el::msglink
