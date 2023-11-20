/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
05.11.23, 18:26
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

This file implements the link class which can be inherited by
the user to define the API/protocol of a link 
(Can be used on client and server side).
*/

#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>

#include <nlohmann/json.hpp>

#include <el/logging.hpp>
#include <el/msglink/event.hpp>
#include <el/msglink/errors.hpp>


namespace el::msglink
{
    /**
     * @brief class that defines the custom communication "protocol"
     * used to communicate with the other party.
     * The link class defines which events, data subscriptions and functions
     * can be transmitted and/or received from/to the other communication party.
     * It is also responsible for defining the data structure and type associated
     * with each of those interactions.
     * 
     * Both clients and server have to define the link in order to be
     * able to communicate. Ideally, the client and server links would match up
     * meaning that for every e.g. event one party can send, the other party knows of
     * and can receive this event.
     * 
     * If a link receives any interaction from the other party that it either doesn't
     * known of, cannot decode or otherwise interpret, an error is automatically
     * reported to the sending party so a language-dependent error handling scheme such
     * as an exception can catch it.
     * 
     */
    class link
    {
    private:

        // 
        bool authentication_done = false;

        // type of the lambda used to wrap event handlers
        using event_handler_wrapper_t = std::function<void(const nlohmann::json &)>;

        // set of all outgoing events (including bidirectional ones)
        std::unordered_set<std::string> outgoing_events;
        // set of all incoming events (including bidirectional ones)
        std::unordered_set<std::string> incoming_events;

        // map of incoming event name to handler function
        std::unordered_map<
            std::string,
            event_handler_wrapper_t
        > incoming_event_handler_map;

    private:    // methods

        /**
         * @brief handles incoming messages (already parsed) before login is complete
         * to perform authentication.
         * 
         * @param _jmsg parsed message
         */
        void handle_message_pre_login(
            const std::string &_msg_type, 
            const int transaction_id,
            const nlohmann::json &_jmsg
        ) {

        }

        /**
         * @brief handles incoming messages (already parsed) after login is complete
         * and parties are authenticated.
         * 
         * @param _jmsg parsed message
         */
        void handle_message_post_login(
            const std::string &_msg_type, 
            const int transaction_id,
            const nlohmann::json &_jmsg
        ) {

        }

    protected:

        /**
         * @brief Method for registering a link-method event handler
         * for a bidirectional event. The event handler must be a method
         * of the link it is registered on. This is a shortcut
         * to avoid having to use std::bind to bind every handler
         * to the instance. When an external handler is needed, this
         * is the wrong overload.
         * 
         * Method function pointer:
         * https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
         * 
         * @tparam _ET the event class of the event to register 
         *             (must inherit from el::msglink::event, can be deduced from method parameter)
         * @tparam _LT the link class the handler is a method of (can also be deduced)
         * @param _handler the handler method for the event
         */
        template <std::derived_from<event> _ET, std::derived_from<link> _LT>
        void define_event(void (_LT::*_handler)(_ET &)) 
        {
            std::string event_name = _ET::_event_name;

            // save to incoming and outgoing event lists
            incoming_events.insert(event_name);
            outgoing_events.insert(event_name);
            
            std::function<void(_LT*, _ET&)> handler = _handler;

            incoming_event_handler_map.emplace(
                _ET::_event_name, 
                [this, handler](const nlohmann::json &_data) {
                    std::cout << "hievent " << _data << std::endl;
                    _ET new_event_inst;
                    new_event_inst = _data;
                    handler(
                        static_cast<_LT*>(this),
                        new_event_inst
                    );
                }
            );

        }


    public:

        /**
         * @brief valid link definitions must implement this define method
         * to define the protocol by calling the specialized define
         * methods for events and other interactions.
         * 
         */
        virtual void define() noexcept = 0;

        void on_message(const std::string &_msg_content)
        {
            try
            {
                nlohmann::json jmsg = nlohmann::json::parse(_msg_content);

                // read message type and transaction ID (always present)
                std::string msg_type = jmsg.at("type");
                int transaction_id = jmsg.at("tid");

                if (authentication_done)
                    handle_message_post_login(
                        msg_type,
                        transaction_id,
                        jmsg
                    );
                else
                    handle_message_pre_login(
                        msg_type,
                        transaction_id,
                        jmsg
                    );
            }
            catch (const nlohmann::json::exception &e)
            {
                throw malformed_message_error(el::strutil::format("Malformed link message: %s\n%s", _msg_content.c_str(), e.what()));
            }
        }
    };

} // namespace el::msglink
