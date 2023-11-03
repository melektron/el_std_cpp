/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
02.11.23, 22:10
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Simple logging framework
*/

#pragma once

#include <string>
#include <cstdarg>
#include <iostream>
#include <memory>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

#include <el/strutil.hpp>


// color escape sequences
#define _EL_LOG_ANSI_COLOR_RED     "\e[31m"
#define _EL_LOG_ANSI_COLOR_GREEN   "\e[32m"
#define _EL_LOG_ANSI_COLOR_YELLOW  "\e[33m"
#define _EL_LOG_ANSI_COLOR_BLUE    "\e[34m"
#define _EL_LOG_ANSI_COLOR_MAGENTA "\e[35m"
#define _EL_LOG_ANSI_COLOR_CYAN    "\e[36m"
#define _EL_LOG_ANSI_COLOR_RESET   "\e[0m"

#define _EL_LOG_PREFIX_BUFFER_SIZE 100

#define _EL_LOG_FILEW 15
#define _EL_LOG_LINEW 4


#define EL_DEFINE_LOGGER() auto logger_inst = el::logging::logger(__FILE__)
#define EL_LOGC(fmt, ...) logger_inst.critical(__LINE__, fmt, ## __VA_ARGS__)
#define EL_LOGE(fmt, ...) logger_inst.error(__LINE__, fmt, ## __VA_ARGS__)
#define EL_LOGW(fmt, ...) logger_inst.warning(__LINE__, fmt, ## __VA_ARGS__)
#define EL_LOGI(fmt, ...) logger_inst.info(__LINE__, fmt, ## __VA_ARGS__)
#define EL_LOGD(fmt, ...) logger_inst.debug(__LINE__, fmt, ## __VA_ARGS__)

#define EL_LOG_EXCEPTION_MSG(msg, ex) EL_LOGE(msg ": %s", el::logging::format_exception(ex).c_str())
#define EL_LOG_EXCEPTION(ex) EL_LOG_EXCEPTION_MSG("Exception occurred", ex)

namespace el::logging
{
    class logger
    {
    private:
        const std::string m_file_name;

        void generate_prefix(char *_output_buffer, int _line, const char *_level)
        {
            snprintf(
                _output_buffer, 
                _EL_LOG_PREFIX_BUFFER_SIZE - 1,
                "[%*.*s@%-*d] %s: ",
                _EL_LOG_FILEW,
                _EL_LOG_FILEW,
                m_file_name.c_str(),
                _EL_LOG_LINEW,
                _line,
                _level
            );
        }

    public:
        logger(std::string _file_name)
            : m_file_name(_file_name)
        {}
        
        // Critical

        void critical(int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _line, "C");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RED << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void critical(int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            critical(_line, strutil::format(_fmt, _args...));
        }

        // Error

        void error(int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _line, "E");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RED << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void error(int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            error(_line, strutil::format(_fmt, _args...));
        }


        // Warning

        void warning(int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _line, "W");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_YELLOW << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void warning(int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            warning(_line, strutil::format(_fmt, _args...));
        }

        // Info

        void info(int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _line, "I");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RESET << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void info(int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            info(_line, strutil::format(_fmt, _args...));
        }

        // Debug

        void debug(int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _line, "D");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_GREEN << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void debug(int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            debug(_line, strutil::format(_fmt, _args...));
        }
        
    };

    /**
     * @brief turns the passed exception into a string
     * in the format "<exception type>: <e.what()>"
     * 
     * Note the type name is demangled in GCC, the raw name (may be demangled
     * depending on impl) is used for other compilers
     * @param  _e the exception to print 
     * @return std::string the printed exception
     */
    std::string format_exception(const std::exception &_e)
    {
#ifdef __GNUC__
        int status = 0;
        char *ex_type_name = abi::__cxa_demangle(typeid(_e).name(), nullptr, nullptr, &status);
        auto output = std::string(ex_type_name) + ": " + _e.what();
        free(ex_type_name);
        return output;
#else
        return std::string(typeid(_e).name()) + ": " + _e.what();
#endif
    }

} // namespace el::log
