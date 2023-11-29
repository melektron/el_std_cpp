/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
26.11.23, 21:17
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

functions to check msglink protocol version compatibility
*/

#pragma once

#include <set>
#include <array>

#include "../../strutil.hpp"


namespace el::msglink::proto_version
{

    using proto_version_t = std::array<uint32_t, 3>;
    
    // the current protocol version of this source tree
    inline const proto_version_t current = {0, 1, 0};

    // all lower protocol versions compatible with this one
    inline const std::set<proto_version_t> compatible_versions =
    {
        {0, 1, 0},
    };

    /**
     * @brief checks if the protocol version _other
     * is compatible with the current protocol version.
     * 
     * @param _other protocol version to check
     * @return true _other is compatible with the current version
     * @return false _other is not compatible with the current version
     */
    bool is_compatible(const proto_version_t &_other)
    {
        return compatible_versions.contains(_other);
    }

    std::string to_string(const proto_version_t &_ver)
    {
        return strutil::format("[%u.%u.%u]", _ver[0], _ver[1], _ver[2]);
    }
} // namespace el::msglink
