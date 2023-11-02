/*
ELEKTRON © 2022 - now
Written by melektron
www.elektron.work
07.10.22, 21:28
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Utility functions operating for strings, mostly STL compatible string types.
*/

#pragma once

#include <stdio.h>
#include <string>
#include <memory>
#include <algorithm>
#include <fstream>
#include <cstring>

namespace el::strutil
{
    /**
     * @brief creates a format string like printf or s(n)printf using the system's
     * snprintf function wrapped in a C++-like interface. 
     * Any STL compatible string type can be used as the format and return value type (see _ST).
     * NOTE: This method uses dynamic memory allocation ("new" operator) and std::unique_ptr.
     * Usually, the template arguments don't have to be provided but can be deducted from function
     * arguments.
     * 
     * @tparam _ST string type of the format and return value. The type must be constructable
     * from "const char *" (string copy) and must have a .c_str() method to convert to "char *"".
     * @tparam _Args varadic format argument types
     * @param _fmt Format string
     * @param _args Format arguments
     * @return _ST newly created string of specified type
     */
    template<typename _ST, typename... _Args>
    _ST format(const _ST& _fmt, _Args... _args)
    {
        size_t len = snprintf(nullptr, 0, _fmt.c_str(), _args...) + 1;  // extra space for null byte
        std::unique_ptr<char[]> _cstr(new char[len]);
        snprintf(_cstr.get(), len, _fmt.c_str(), _args...);
        return _ST(_cstr.get());
    }

    /**
     * @brief creates a copy of a string with all lowercase letters.
     * The tolower() C function is used to convert the letters. 
     * Any string class compatible with the C++ std::string class in terms
     * of iteration and uses "char" as the character type can be used.
     * 
     * @tparam _ST string type to be used (deducted, typically std::string)
     * @param instr the input string to convert
     * @return _ST copy of the string in lowercase
     */
    template<typename _ST>
    _ST lowercase(_ST instr)
    {
        std::for_each(instr.begin(), instr.end(), [](char &c)
                    { c = ::tolower(c); });

        return instr;
    }

    /**
     * @brief creates a copy of a string with all lowercase letters.
     * The tolower() C function is used to convert the letters. 
     * Any string class compatible with the C++ std::string class in terms
     * of iteration and uses "char" as the character type can be used.
     * 
     * @tparam _ST string type to be used (deducted, typically std::string)
     * @param instr the input string to convert
     * @return _ST copy of the string in lowercase
     */
    template<typename _ST>
    _ST uppercase(_ST instr)
    {
        std::for_each(instr.begin(), instr.end(), [](char &c)
                    { c = ::toupper(c); });

        return instr;
    }

    /**
     * @brief Reads the entire content of a file and stores it in a string.
     * @exception This function can trough any exception that the string or ifstream can.
     * 
     * @tparam _ST string type, typically std::string (can be deducted)
     * @param _file The file stream to read from
     * @param _string The string to store the file contents in. This will overwrite the string.
     * @return The length of the file (= the number of characters copied to the string)
     */
    template<typename _ST>
    size_t read_file_into_string(std::ifstream &_file, _ST &_string)
    {
        // get file length
        _file.seekg(0, std::ios::end);
        std::streampos length = _file.tellg();
        _file.seekg(0, std::ios::beg);

        _string.reserve(length);
        _string.assign( (std::istreambuf_iterator<char>(_file)  ), 
                        (std::istreambuf_iterator<char>()       ));
                    
        return length;
    }


    /**
     * @brief stringswitch - a macro based wrapper for if statements
     * allowing you to compare std::strings using syntax somewhat similar to 
     * switch-case statements. As this is purly macro based, there will be
     * no namespace annotations unfortunately.
     * 
     * Limitations: variables created inside the block are local, every case
     * has to use brackets if it is more than one statement in size
     */

#define stringswitch(strval) {\
    const std::string &__el_strswitch_strtempvar__ = strval;

#define scase(strval) if (__el_strswitch_strtempvar__ == strval)

#define switchend }


};