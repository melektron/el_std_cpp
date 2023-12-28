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
     * Inspiration: https://stackoverflow.com/a/26221725
     * 
     * @tparam _Args varadic format argument types
     * @param _fmt Format string
     * @param _args Format arguments
     * @return newly created string of specified type
     */
    template<typename... _Args>
    std::string format(const std::string& _fmt, _Args... _args)
    {
        int len_or_error = std::snprintf(nullptr, 0, _fmt.c_str(), _args...) + 1;  // extra space for null byte
        
        if( len_or_error <= 0 )
            throw std::runtime_error( "Error during formatting." );
        auto len = static_cast<size_t>(len_or_error);
        
        std::unique_ptr<char[]> _cstr(new char[len]);
        std::snprintf(_cstr.get(), len, _fmt.c_str(), _args...);
        
        return std::string(_cstr.get());
    }

    /**
     * @brief creates a copy of a string with all lowercase letters.
     * The tolower() C function is used to convert the letters. 
     * 
     * @param instr the input string to convert
     * @return copy of the string in lowercase
     */
    inline std::string lowercase(std::string instr)
    {
        std::for_each(instr.begin(), instr.end(), [](char &c)
                    { c = ::tolower(c); });

        return instr;
    }

    /**
     * @brief creates a copy of a string with all lowercase letters.
     * The tolower() C function is used to convert the letters. 
     * 
     * @param instr the input string to convert
     * @return copy of the string in lowercase
     */
    inline std::string uppercase(std::string instr)
    {
        std::for_each(instr.begin(), instr.end(), [](char &c)
                    { c = ::toupper(c); });

        return instr;
    }

    /**
     * @brief Reads the entire content of a file and stores it in a string.
     * @exception This function can trough any exception that the string or ifstream can.
     * 
     * @param _file The file stream to read from
     * @param _string The string to store the file contents in. This will overwrite the string.
     * @return The length of the file (= the number of characters copied to the string)
     */
    inline size_t read_file_into_string(std::ifstream &_file, std::string &_string)
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
     * @brief chain compare - a macro based wrapper for if statements
     * allowing you to compare arbitrary types using syntax somewhat similar to 
     * switch-case statements. As this is purely macro based, there will generate
     * if else statements comparing values. 
     * 
     * Limitations: variables created inside the block are local, every case
     * has to use brackets if it is more than one statement in size
     */
#define el_chain_compare(variable)                          \
    {                                                       \
        const auto &__el_strswitch_strtempvar__ = variable; \
        if (false) {}

#define el_case(value) else if (__el_strswitch_strtempvar__ == value)

#define el_end_compare }

};