/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
06.10.22, 21:50
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

universal_t is a universal object, like a variable of dynamic type.

This header requires el::strutil (strutil.hpp). It can only be used if the 
platform supports the required functions and standard headers.
*/

#pragma once

#include <string>
#include <iostream>

#include "strutil.hpp"
#include "types.hpp"

namespace el
{
    class universal
    {
    protected:

        /**
         * @brief enumeration for the type currently stored in the 
         * universal data container.
         */
        enum class type_t
        {
            empty,
            string,
            integer, // integer is 64 bit
            floating, // double (64 bit)
            boolean,
            rgb24
        };

        /**
         * @brief structure containing the actual data 
         * contained in the universal data container.
         */
        struct data_t
        {
            std::string string;
            int64_t integer;
            double floating;
            types::rgb24_t rgb24;
            bool boolean;
        };
        // current type of the universal container
        type_t type = type_t::empty;

        // current content of the universal container
        data_t data;

        /**
         * @brief the unit is an additional string identifying the unit of
         * the data contained in the container, no matter of the type.
         * This is completely independent on the type and value and has no
         * effect on type conversions and comparisons. It is simply additional information
         * that can be set and accessed by the user code.
         */
        std::string unit = "";


        // == Settings (flags) that might become configurable in the future == //
        // whether string should be cleared when switching to another type. This adds memory allocation and deallocation time but saves on memory
        bool conf_clear_string = true;

    protected:    // methods
        /**
         * @brief method to clear the contents of the internal string if the current
         * type is string and string clearing is enabled
         * 
         */
        void clear_string()
        {
            if (!conf_clear_string) return;

            if (type == type_t::string)
                data.string.clear();
        }
    
    public:

        // == constructors to initialize with any data type == //
        
        // empty initialization empty (no data)
        universal() = default;  

        // string initialization     
        universal(const std::string &_d) { type = type_t::string; data.string = _d; };
        // c-string and string literal initialization.
        // (used for string literals and c-strings that would otherwise result in the bool overload being chosen)
        universal(const char *_d) { type = type_t::string; data.string = _d; };
        // integer initialization
        universal(int64_t _d) { type = type_t::integer; data.integer = _d; };
        // floating point value initialization
        universal(double _d) { type = type_t::floating; data.floating = _d; };
        // boolean initialization
        universal(bool _d) { type = type_t::boolean; data.boolean = _d; };
        // 24 bit RGB color initialization
        universal(const types::rgb24_t &_d)  { type = type_t::rgb24; data.rgb24 = _d; };

        /*
         * operator= to set any of the data values
         * it is advised to assign using operator= instead of
         * directly assigning the data container because the type will be
         * updated by the operator.
        */
        // string copy assignment
        universal &operator=(const std::string &_d)
        {
            type = type_t::string;
            data.string = _d;
            return *this;
        }
        // string move assignment
        universal &operator=(std::string &&_d)
        {
            type = type_t::string;
            data.string = std::move(_d);
            return *this;
        }
        // c-string and string literal (copy) assignment.
        // (used for string literals and c-strings that would otherwise result in the bool overload being chosen)
        universal &operator=(const char *_d)
        {
            type = type_t::string;
            data.string = _d;
            return *this;
        }
        // integer assignment
        universal &operator=(int64_t _d)
        {
            clear_string();
            type = type_t::integer;
            data.integer = _d;
            return *this;
        }
        // floating point assignment
        universal &operator=(double _d)
        {
            clear_string();
            type = type_t::floating;
            data.floating = _d;
            return *this;
        }
        // boolean assignment
        universal &operator=(bool _d)
        {
            clear_string();
            type = type_t::boolean;
            data.boolean = _d;
            return *this;
        }
        // rgb color assignment
        universal &operator=(const types::rgb24_t &_d)
        {
            clear_string();
            type = type_t::rgb24;
            data.rgb24 = _d;
            return *this;
        }

        // == standard display operators == //

        // stream operator to output data
        friend std::ostream &operator<<(std::ostream &_os, const universal &_data)
        {
            switch (_data.type)
            {
            case universal::type_t::empty:
                _os << "(empty)";
                break;

            case universal::type_t::string:
                _os << _data.data.string;
                break;

            case universal::type_t::integer:
                _os << _data.data.integer;
                break;

            case universal::type_t::floating:
                _os << _data.data.floating;
                break;

            case universal::type_t::boolean:
                _os << (_data.data.boolean ? "true" : "false");
                break;

            case universal::type_t::rgb24:
                _os << el::strutil::format<std::string>("(%d, %d, %d)", _data.data.rgb24.r, _data.data.rgb24.g, _data.data.rgb24.b);
                break;

            default:
                break;
            }

            return _os;
        }

        // == operators to compare universal data structure == //

        // base equals comparator between two universal data structures
        friend bool operator==(const universal &lhs, const universal &rhs)
        {
            // call type specific operators of lhs depending on type of rhs
            switch (rhs.type)
            {
            case type_t::string:
                return lhs == rhs.data.string;
                break;

            case type_t::integer:
                return lhs == rhs.data.integer;
                break;

            case type_t::floating:
                return lhs == rhs.data.floating;
                break;

            case type_t::boolean:
                return lhs == rhs.data.boolean;
                break;

            case type_t::rgb24:
                return lhs == rhs.data.rgb24;
                break;

            case type_t::empty:
                return lhs.type == type_t::empty;

            // invalid type returns false
            default:
                break;
            }

            return false;
        }

        // comparison operators for comparing to raw types (used by base operator)
        friend bool operator==(const universal &lhs, const std::string &rhs)
        {
            if (lhs.type != type_t::string) return false;   // string is never equal to anything other than a string
            return lhs.data.string == rhs;                  // if it is a string, compare the strings
        }
        friend bool operator==(const universal &lhs, const int64_t rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return lhs.data.integer == rhs;
                break;

            case type_t::floating:
                return lhs.data.floating == (double)rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean == (bool)rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, treat integer as 24 bit packet rgb color
                return lhs.data.rgb24 == (types::rgb24_t)rhs;
                break;

            case type_t::string:
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator==(const universal &lhs, const double rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return (double)(lhs.data.integer) == rhs;
                break;

            case type_t::floating:
                return lhs.data.floating == rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean == (bool)rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, convert double to integer and treat as 24 bit packed rgb color
                return lhs.data.rgb24 == (types::rgb24_t)rhs;
                break;

            case type_t::string:
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator==(const universal &lhs, const bool rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return lhs.data.integer == (int64_t)rhs;
                break;

            case type_t::floating:
                return lhs.data.floating == (double)rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean == rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, treat boolean 1 as white and 0 as black
                return lhs.data.rgb24 == (types::rgb24_t)(rhs ? 0xffffff : 0x0);
                break;

            case type_t::string:
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator==(const universal &lhs, const types::rgb24_t &rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return (types::rgb24_t)lhs.data.integer == rhs;
                break;

            case type_t::floating:
                return (types::rgb24_t)lhs.data.floating == rhs;
                break;

            case type_t::boolean:
                // when comparing to boolean, treat boolean 1 as white and 0 as black
                return (types::rgb24_t)(lhs.data.boolean ? 0xffffff : 0x0) == rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, treat integer as 24 bit packet rgb color
                return lhs.data.rgb24 == rhs;
                break;

            case type_t::string:
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        
        // base less than comparator between two universal data structures
        friend bool operator<(const universal &lhs, const el::universal &rhs)
        {
            // call type specific operators of lhs depending on type of rhs
            switch (rhs.type)
            {
            case type_t::string:
                return lhs < rhs.data.string;
                break;

            case type_t::integer:
                return lhs < rhs.data.integer;
                break;

            case type_t::floating:
                return lhs < rhs.data.floating;
                break;

            case type_t::boolean:
                return lhs < rhs.data.boolean;
                break;

            case type_t::rgb24:
                return lhs < rhs.data.rgb24;
                break;

            case type_t::empty:
                return lhs.type == type_t::empty;

            // invalid type returns false
            default:
                break;
            }

            return false;
        }

        // less than operators for comparing raw types to universal data structure (used by base operator)
        friend bool operator<(const universal &lhs, const std::string &rhs)
        {
            // when comparing strings, use their length
            return lhs < (int64_t)rhs.length();
        }
        friend bool operator<(const universal &lhs, const int64_t rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return lhs.data.integer < rhs;
                break;

            case type_t::floating:
                return lhs.data.floating < rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean < rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, compare the brightness
                return lhs.data.rgb24.get_brightness() < rhs;
                break;

            case type_t::string:
                // compare string length
                return lhs.data.string.length() < rhs;
            
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator<(const universal &lhs, const double rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return lhs.data.integer < rhs;
                break;

            case type_t::floating:
                return lhs.data.floating < rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean < rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, compare the brightness
                return lhs.data.rgb24.get_brightness() < rhs;
                break;

            case type_t::string:
                // compare string length
                return lhs.data.string.length() < rhs;
            
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator<(const universal &lhs, const bool rhs)
        {
            switch (lhs.type)
            {
            
            case type_t::integer:
                return lhs.data.integer < rhs;
                break;

            case type_t::floating:
                return lhs.data.floating < rhs;
                break;

            case type_t::boolean:
                return lhs.data.boolean < rhs;
                break;

            case type_t::rgb24:
                // when comparing to color, compare the brightness
                return lhs.data.rgb24.get_brightness() < rhs;
                break;

            case type_t::string:
                // compare string length
                return lhs.data.string.length() < rhs;
            
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        friend bool operator<(const universal &lhs, const types::rgb24_t &rhs)
        {
            // color comares using brightness
            switch (lhs.type)
            {
            case type_t::integer:
                return lhs.data.integer < rhs.get_brightness();
                break;

            case type_t::floating:
                return lhs.data.floating < rhs.get_brightness();
                break;

            case type_t::boolean:
                return lhs.data.boolean < rhs.get_brightness();
                break;

            case type_t::rgb24:
                // when comparing to color, compare the brightness
                return lhs.data.rgb24.get_brightness() < rhs.get_brightness();
                break;

            case type_t::string:
                // compare string length
                return lhs.data.string.length() < rhs.get_brightness();
            
            case type_t::empty:
            default:    // invalid type returns false
                return false;
            }
        }
        
        // compare operators derived from one of the above
        friend inline bool operator!=(const universal &lhs, const universal &rhs) { return !(lhs == rhs); }
        friend inline bool operator>(const universal &lhs, const universal &rhs) { return rhs < lhs; };
        friend inline bool operator<=(const universal &lhs, const universal &rhs) { return !(lhs > rhs); };
        friend inline bool operator>=(const universal &lhs, const universal &rhs) { return !(lhs < rhs); };
    };
};

