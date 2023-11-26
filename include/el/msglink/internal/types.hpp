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
#include <array>

namespace el::msglink
{
    using tid_t = int64_t;
    using proto_version_t = std::array<uint32_t, 3>;
    using link_version_t = uint32_t;
} // namespace el::msglink
