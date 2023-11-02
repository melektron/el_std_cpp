/*
ELEKTRON © 2022 - now
Written by melektron
www.elektron.work
20.11.22, 21:43
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

This header adds a std::hash implementation for std::filesystem::path. It also adds
el::path as a shorter alias for std::filesystem::path
*/

#pragma once

#include <filesystem>

namespace el
{
    using path = std::filesystem::path;
};

// std::hash specialization of the el::path structure required for unordered map/set keys
// Inspiration: https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
namespace std
{
    template <>
    struct hash<el::path>
    {
        std::size_t operator()(const el::path &_path) const
        {
            return std::hash<const el::path::value_type*>()(_path.c_str());
        }
    };
};
