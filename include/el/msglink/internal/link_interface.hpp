/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
29.11.23, 08:46
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Class used as a common interface for the underlying networking class
used by the link instance to communicate to the other party.

Both server and client networking implementations inherit from this and provide
the common interface declared by this class.
*/

#pragma once

#include <string>
#include <nlohmann/json.hpp>


namespace el::msglink
{
    class link_interface
    {
    public:
        /**
         * @brief closes the connection and possibly destroys the link
         * (depending on the function of the communication backend, e.g.
         * server vs. client)
         * 
         * @param _code the close code specifying the error/reason causing the close.
         * @param _reason human readable reason for close.
         */
        virtual void close_connection(int _code, std::string _reason) noexcept = 0;

        /**
         * @brief encodes and then sends json content through the 
         * communication channels
         * 
         * @param _jcontent json document to send
         */
        void send_message(const nlohmann::json &_jcontent)
        {
            send_message(_jcontent.dump());
        };

    protected:
        /**
         * @brief sends a message via the communication channel.
         * 
         * @param _content string content to send
         */
        virtual void send_message(const std::string &_content) = 0;
    };
} // namespace el::msglink
