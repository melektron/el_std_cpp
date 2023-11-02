/*
ELEKTRON © 2022 - now
Written by melektron
www.elektron.work
19.11.22, 23:57
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

utility functions for the nlohmann json library. 

This depends on the nlohmann::json library
which must be includeable like this: "#include <nlohmann/json.hpp>"
*/

#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "cxxversions.h"
#ifdef __EL_ENABLE_CXX17
#include <optional>
#endif


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
        catch(const nlohmann::json::exception& e)
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
        catch(const nlohmann::json::exception& e)
        {
            return _default;
        }
        
    }

#ifdef __EL_ENABLE_CXX17
    /**
     * @brief reads a key from a provided json object and returns it's value if it 
     * exists and is convertable to the desired type, an empty optional otherwise.
     * 
     * @tparam _T desired output type
     * @param _jobj json object
     * @param _key key in json object
     * @return value of provided type or nothing
     */
    template <typename _T>
    std::optional<_T> json_or_nothing(const nlohmann::json &_jobj, const std::string &_key)
    {
        try
        {
            return _jobj.at(_key).get<_T>();
        }
        catch(const nlohmann::json::exception& e)
        {
            return {};
        }
        
    }

    /**
     * @brief tries to convert a json object to the desired type and returns it's value
     * or an empty optional if the conversion is not possible.
     * 
     * @tparam _T desired type
     * @param _jobj json object to convert
     * @param _default default value
     * @return value of provided type or nothing
     */
    template <typename _T>
    std::optional<_T> json_or_nothing(const nlohmann::json &_jobj)
    {
        try
        {
            return _jobj.get<_T>();
        }
        catch(const std::exception& e)
        {
            return {};
        }
        
    }

#endif

    /**
     * @brief reads a key from a provided json object if it exists and checks
     * it's equality to a provided value. If the key does not exist, could not 
     * be converted to the desired type or the value is not equal to the provided one,
     * the function returns false. Note that operator==() must be implemented for the 
     * desired type in order for this function to work.
     * 
     * @tparam _T desired value type (can be deducted in some cases)
     * @param _jobj json object
     * @param _key key in json object
     * @param _value value to compare to
     * @return boolean
     * @retval true The data exists and is equal
     * @retval false The data doesn't exist or is not equal
     */
    template <typename _T>
    bool json_check(const nlohmann::json &_jobj, const std::string &_key, const _T &_value)
    {
        try
        {
            return _jobj.at(_key).get<_T>() == _value;
        }
        catch(const nlohmann::json::exception& e)
        {
            return false;
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
    static bool json_validate(const nlohmann::json &_jobj, const std::string &_key, json_type_t _type)
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
    static bool json_validate(const nlohmann::json &_jarr, int _index, json_type_t _type)
    {
        if (!_jarr.is_array()) return false;
        if (!(_jarr.size() > _index)) return false;
        if (_jarr.at(_index).type() != _type) return false;
        return true;
    }
};