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

#include "cxxversions.h"

#include <string>
#include <cstdarg>
#include <iostream>
#include <memory>

#ifdef __EL_ENABLE_CXX20
#include <source_location>
#endif

#include "strutil.hpp"
#include "rtti_utils.hpp"


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

#if defined(__EL_PLATFORM_LINUX)
#   include <linux/limits.h>
#elif defined(__EL_PLATFORM_WINDOWS)
#   include <windows.h>
#   define PATH_MAX MAX_PATH
#elif defined(__EL_PLATFORM_APPLE)
#   include <sys/syslimits.h>
#endif

/**
 * Source location information.
 * Since C++20 there is builtin portable support for getting
 * the source location by using the <source_location> header.
 * It provides the std::source_location type as well as the function 
 * std::source_location::current() to get the current source location.
 * The returned object can be used to retrieve file name, line number, column
 * and function name.
 * 
 * Before C++20, macros and other builtin symbols had to be used. This is not quite
 * portable however.
 * 
 * We use the C++20 features where possible and fallback to macros otherwise.
 */

#ifdef __EL_ENABLE_CXX20

#define _EL_LOG_FILE std::source_location::current().file_name()
#define _EL_LOG_LINE std::source_location::current().line()
#define _EL_LOG_FUNCTION std::source_location::current().function_name()

#else

// these might not be possible for every compiler
#define _EL_LOG_FILE __FILE__
#define _EL_LOG_LINE __LINE__
#define _EL_LOG_FUNCTION __PRETTY_FUNCTION__

#endif


#define EL_DEFINE_LOGGER() el::logging::logger el::logging::logger_inst
#define EL_LOGC(fmt, ...) el::logging::logger_inst.critical(_EL_LOG_FILE, _EL_LOG_LINE, fmt, ## __VA_ARGS__)
#define EL_LOGE(fmt, ...) el::logging::logger_inst.error(_EL_LOG_FILE, _EL_LOG_LINE, fmt, ## __VA_ARGS__)
#define EL_LOGW(fmt, ...) el::logging::logger_inst.warning(_EL_LOG_FILE, _EL_LOG_LINE, fmt, ## __VA_ARGS__)
#define EL_LOGI(fmt, ...) el::logging::logger_inst.info(_EL_LOG_FILE, _EL_LOG_LINE, fmt, ## __VA_ARGS__)
#define EL_LOGD(fmt, ...) el::logging::logger_inst.debug(_EL_LOG_FILE, _EL_LOG_LINE, fmt, ## __VA_ARGS__)

#define EL_LOG_EXCEPTION_MSG(msg, ex) EL_LOGE(msg ": %s", el::logging::format_exception(ex).c_str())
#define EL_LOG_EXCEPTION(ex) EL_LOG_EXCEPTION_MSG("Exception occurred", ex)

#define EL_LOG_FUNCTION_CALL() EL_LOGD("Function call: \e[1;3m%s\e", _EL_LOG_FUNCTION)    // the color reset happens at end of line anyway, bold is reset there too 


namespace el::logging
{
    class logger
    {
    private:

        void generate_prefix(char *_output_buffer, const char *_file, int _line, const char *_level)
        {
            // A file name/path can be (_EL_LOG_FILEW - 1) characters long and will be 
            // printed in a _EL_LOG_FILEW characters wide area, so ther will always be a space
            // to the left side. If the path is _EL_LOG_FILEW or more characters long, then it will
            // be truncated on the left side and the space will be replaced by an overflow indicator ('<')

            // substring filename for alignment if necessary
            char file_name[_EL_LOG_FILEW + 1] = {'\0'};
            size_t file_len = strnlen(_file, PATH_MAX);
            if (file_len != PATH_MAX && file_len > (_EL_LOG_FILEW - 1)) // bigger than width -1 because we always want to have one space to the left unless for the overflow indicator
            {
                // if filename is to wide, truncate it to the _EL_LOG_FILEW rightmost characters
                // and add '<' to it's start
                strncpy(file_name, _file + (file_len - _EL_LOG_FILEW), _EL_LOG_FILEW);
                file_name[_EL_LOG_FILEW] = '\0';
                file_name[0] = '<';
            }
            else
            {
                // otherwise just copy it
                strncpy(file_name, _file, _EL_LOG_FILEW);
                file_name[_EL_LOG_FILEW] = '\0';
            }

            snprintf(
                _output_buffer, 
                _EL_LOG_PREFIX_BUFFER_SIZE - 1,
                "[%*.*s@%-*d ] %s: ",  // at least one space on the side always
                _EL_LOG_FILEW,
                _EL_LOG_FILEW,
                file_name,
                _EL_LOG_LINEW,
                _line,
                _level
            );
        }

    public:
        logger() = default;
        
        // Critical

        void critical(const char *_file, int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _file, _line, "C");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RED << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void critical(const char *_file, int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            critical(_file, _line, strutil::format(_fmt, _args...));
        }

        // Error

        void error(const char *_file, int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _file, _line, "E");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RED << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void error(const char *_file, int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            error(_file, _line, strutil::format(_fmt, _args...));
        }


        // Warning

        void warning(const char *_file, int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _file, _line, "W");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_YELLOW << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void warning(const char *_file, int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            warning(_file, _line, strutil::format(_fmt, _args...));
        }

        // Info

        void info(const char *_file, int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _file, _line, "I");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_RESET << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void info(const char *_file, int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            info(_file, _line, strutil::format(_fmt, _args...));
        }

        // Debug

        void debug(const char *_file, int _line, const std::string &_message)
        {
            // generate prefix
            char prefix_buffer[_EL_LOG_PREFIX_BUFFER_SIZE];
            generate_prefix(prefix_buffer, _file, _line, "D");

            // print in color
            std::cout << _EL_LOG_ANSI_COLOR_GREEN << prefix_buffer << _message << _EL_LOG_ANSI_COLOR_RESET << std::endl;
        }

        template<typename... _Args>
        void debug(const char *_file, int _line, const std::string &_fmt, _Args... _args)
        {
            // format the message
            debug(_file, _line, strutil::format(_fmt, _args...));
        }
        
    };

    // declaration of global logger instance which has to be defined by the user
    extern logger logger_inst;

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
        return rtti::demangle_if_possible(typeid(_e).name()) + "\n  what():  " + _e.what();
    }

} // namespace el::log
