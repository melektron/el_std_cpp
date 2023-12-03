/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
02.11.23, 23:23
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

el-std base exceptions
*/

#pragma once

#include <exception>
#include <string>

#include "strutil.hpp"


namespace el
{
    /**
     * @brief el-std base exception allowing custom messages
     */
    class exception : public std::exception
    {
    private:
        std::string m_message;
    
    public:
        exception(const char *_msg)
            : m_message(_msg)
        {}

        template<typename... _Args>
        exception(const std::string &_msg_fmt, _Args... _args)
            : m_message(strutil::format(_msg_fmt, _args...))
        {}

        virtual ~exception() noexcept = default;

        virtual const char *what() const noexcept override
        {
            return m_message.c_str();
        }
    };

} // namespace el

