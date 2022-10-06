/*
ELEKTRON © 2022
Written by Matteo Reiter
www.elektron.work
06.10.22, 21:50
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

universal_t is a universal object, like a variable of dynamic type
*/

#pragma once

#include <string>

namespace el
{
    class universal
    {
    private:

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
            struct rgb24_t
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
            } rgb24;
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
        universal(const data_t::rgb24_t &_d)  { type = type_t::rgb24; data.rgb24 = _d; };

        // operator= to set any of the data values
        // it is advised to assign using operator= instead of
        // directly assigning the data container because the type will be
        // updated by the operator
        universal &operator=(const std::string &_d);     // string
        universal &operator=(const char *_d);            // string (used for literal strings and c-strings same reason as above)
        universal &operator=(int64_t _d);                // integer
        universal &operator=(double _d);                 // floating
        universal &operator=(bool _d);                   // boolean
        universal &operator=(const data_t::rgb24_t &_d); // color

        // stream operator to output data
        friend std::ostream &operator<<(std::ostream &_os, const universal &_data);

        // == operators to compare universal data structure == //

        // base equals comparator between two event data structures
        friend bool operator==(const el::universal &lhs, const el::universal &rhs);

        // comparison operators for comparing to raw types (used by base operator)
        friend bool operator==(const el::universal &lhs, const std::string &rhs);
        friend bool operator==(const el::universal &lhs, const int64_t rhs);
        friend bool operator==(const el::universal &lhs, const double rhs);
        friend bool operator==(const el::universal &lhs, const bool rhs);
        friend bool operator==(const el::universal &lhs, const el::universal::data_t::rgb24_t &rhs);
        friend bool operator<(const el::universal &lhs, const el::universal &rhs);

        // compare operators derived from one of the above
        friend inline bool operator!=(const el::universal &lhs, const el::universal &rhs) { return !(lhs == rhs); }
        friend inline bool operator>(const el::universal &lhs, const el::universal &rhs) { return rhs < lhs; };
        friend inline bool operator<=(const el::universal &lhs, const el::universal &rhs) { return !(lhs > rhs); };
        friend inline bool operator>=(const el::universal &lhs, const el::universal &rhs) { return !(lhs < rhs); };
    };
};

