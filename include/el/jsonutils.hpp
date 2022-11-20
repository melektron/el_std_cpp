/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
19.11.22, 23:57
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

utility functions for the nlohmann json library. 

This depends on the nlohmann::json library
which must be includable like this: "#include <nlohmann-json/json.hpp>"
*/

#pragma once

#include <nlohmann-json/json.hpp>
#include <string>

namespace el
{
    // alias for the json value type to make it shorter
    using json_type_t = nlohmann::json::value_t;


    /**
     * @brief reads a key from a provided json object and returns it's value if it 
     * exists and is convertable to the desired type, the default value otherwise.
     * 
     * @tparam _T desired output type
     * @param _jobj json object
     * @param _key key in json object
     * @param _default default value if key is not accessible
     * @return value of provided type
     */
    template <typename _T>
    _T json_or_default(const nlohmann::json &_jobj, const std::string &_key, const _T &_default)
    {
        try
        {
            return _jobj.at(_key).get<_T>();
        }
        catch(const std::exception& e)
        {
            return _default;
        }
        
    }

    /**
     * @brief tries to convert a json object to the desired type and returns it's value
     * or the default value if the conversion is not possible.
     * 
     * @tparam _T desired type
     * @param _jobj json object to convert
     * @param _default default value
     * @return value of provided type
     */
    template <typename _T>
    _T json_or_default(const nlohmann::json &_jobj, const _T &_default)
    {
        try
        {
            return _jobj.get<_T>();
        }
        catch(const std::exception& e)
        {
            return _default;
        }
        
    }

    /**
     * @brief returns true if the key _key of json object _jobj exists
     * is of type _type. If _jobj is not an object or does not contain
     * a filed called _key, false is returned
     * 
     * @param _jobj json object to validate
     * @param _key key to check for
     * @param _type type of the key
     * @return true exists and correct type
     * @return false doesn't exist or is not of correct type
     */
    bool json_validate(const nlohmann::json &_jobj, const std::string &_key, json_type_t _type)
    {
        if (!_jobj.is_object()) return false;
        if (!_jobj.contains(_key)) return false;
        if (_jobj.at(_key).type() != _type) return false;
        return true;
    }

    /**
     * @brief returns true if the index _index of json array _jobj exists
     * is of type _type. If _jarr is not an array or does not contain
     * an _index-th element, false is returned.
     * 
     * This can be used to check that the array contains the correct type of data
     * and has a required length.
     * 
     * @param _jarr json array to validate
     * @param _index index to check for
     * @param _type type of the key
     * @return true exists and correct type
     * @return false doesn't exist or is not of correct type
     */
    bool json_validate(const nlohmann::json &_jarr, int _index, json_type_t _type)
    {
        if (!_jarr.is_array()) return false;
        if (!(_jarr.size() > _index)) return false;
        if (_jarr.at(_index).type() != _type) return false;
        return true;
    }
};