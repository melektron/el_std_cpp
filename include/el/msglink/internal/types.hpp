/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
26.11.23, 21:10
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink type aliases used in other files to easily be able to change types
*/

#pragma once

#include <cstdint>

#include "proto_version.hpp"


namespace el::msglink
{
    using tid_t = int64_t;      // transaction ID
    using sub_id_t = int64_t;   // subscription ID
    using proto_version::proto_version_t;
    using link_version_t = uint32_t;
} // namespace el::msglink
