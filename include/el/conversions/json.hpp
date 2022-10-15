/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
15.10.22, 14:34
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Conversion operators from other structures to nlohmann::json. This depends on the nlohmann::json library
which must be includable like this: "#include <nlohmann-json/json.hpp>"
*/

#pragma once

#include <nlohmann-json/json.hpp>
#include "../universal.hpp"



namespace el
{
    nlohmann::json universal_to_json(const universal &_data)
    {
        if (_data.get_type() == universal::type_t::string)
            return _data.to_string();
        else if (_data.get_type() == universal::type_t::integer)
            return _data.to_int64_t();
        else if (_data.get_type() == universal::type_t::floating)
            return _data.to_double();
        else if (_data.get_type() == universal::type_t::boolean)
            return _data.to_bool();
        else if (_data.get_type() == universal::type_t::rgb24)
        {
            auto content = _data.to_rgb24_t();
            return {
                {"r", content.r},
                {"g", content.g},
                {"b", content.b},
            };
        }
        return {};
    }
};