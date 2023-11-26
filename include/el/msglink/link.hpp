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

#include "../logging.hpp"
#include "../flags.hpp"
#include "event.hpp"
#include "errors.hpp"
#include "internal/msgtype.hpp"
#include "internal/messages.hpp"
#include "internal/types.hpp"
#include "internal/proto_version.hpp"


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

        // the value to step for every new transaction ID generated.
        // This is set to either 1 or -1 in the constructor depending on wether
        // it is being used by a server or a client
        const int8_t tid_step_value;
        // running counter for transaction IDs (initialized to tid_step_value)
        tid_t tid_counter;

        // flags set to track the authentication process
        soflag auth_ack_sent;
        soflag auth_ack_received;
        // set as soon as the login_ack has been sent and received
        soflag authentication_done;

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

        decltype(link::tid_counter) generate_new_tid() noexcept
        {
            // use value before counting, so first value is 1/-1
            // as defined in the spec
            auto tmp = tid_counter;
            tid_counter += tid_step_value;
            return tid_counter;
        }

        /**
         * @brief updates authentication done flag from
         * the other authentication flags' values
         */
        void update_auth_done() noexcept
        {
            if (auth_ack_sent && auth_ack_received)
                authentication_done.set();
        }

        /**
         * @return link_version_t link version of the link instance
         */
        link_version_t get_link_version() const noexcept
        {
            return _el_msglink_get_link_version();
        }

        /**
         * @brief handles incoming messages (already parsed) before authentication is complete
         * to perform the authentication.
         *
         * @param _jmsg parsed message
         */
        void handle_message_pre_auth(
            const msg_type_t _msg_type,
            const int transaction_id,
            const nlohmann::json &_jmsg
        )
        {
            switch (_msg_type)
            {
                using enum msg_type_t;

            case AUTH:
            {
                // validate message
                msg_auth_t msg(_jmsg);

                // check protocol version if we are the higher one
                if (proto_version::current > msg.proto_version)
                    if (!proto_version::is_compatible(msg.proto_version))
                        throw incompatible_link_error(el::strutil::format(
                            "Incompatible protocol versions: this=%u, other=%u",
                            proto_version::to_string(proto_version::current).c_str(),
                            proto_version::to_string(msg.proto_version).c_str()
                        ));

                // check user defined link version
                if (msg.link_version != get_link_version())
                    throw incompatible_link_error(el::strutil::format("Link versions don't match: this=%u, other=%u", get_link_version(), msg.link_version));
            }
            break;

            case AUTH_ACK:
            {
                msg_auth_ack_t msg(_jmsg);


            }
            break;

            default:
                throw malformed_message_error(el::strutil::format("Invalid pre-auth message type: %s", msg_type_to_string(_msg_type)));
                break;
            }

            update_auth_done();
        }

        /**
         * @brief handles incoming messages (already parsed) after authentication is complete
         * and both parties are authenticated.
         *
         * @param _jmsg parsed message
         */
        void handle_message_post_auth(
            const msg_type_t _msg_type,
            const int transaction_id,
            const nlohmann::json &_jmsg
        )
        {
            switch (_msg_type)
            {
                using enum msg_type_t;

            case EVENT_SUB:
                break;
            case EVENT_SUB_ACK:
                break;
            case EVENT_SUB_NAK:
                break;
            case EVENT_UNSUB:
                break;
            case EVENT_EMIT:
                break;
            case DATA_SUB:
                break;
            case DATA_SUB_ACK:
                break;
            case DATA_SUB_NAK:
                break;
            case DATA_UNSUB:
                break;
            case DATA_CHANGE:
                break;
            case RPC_CALL:
                break;
            case RPC_NAK:
                break;
            case RPC_ERR:
                break;
            case RPC_RESULT:
                break;

            default:
                throw malformed_message_error(el::strutil::format("Invalid post-auth message type: %s", msg_type_to_string(_msg_type)));
                break;
            }

        }

    protected:

        /**
         * @return int64_t user defined link version.
         * Use the EL_LINK_VERSION macro to generate this function.
         */
        virtual link_version_t _el_msglink_get_link_version() const noexcept = 0;

        /**
         * @brief Macro used to define the user defined link version inside the link
         * definitions. This simply creates a method returning the provided number.
         *
         * The link version is an integer that defined the version of the
         * user defined protocol the link represents. When two parties connect, their
         * link versions must match.
         */
#define EL_MSGLINK_LINK_VERSION(version_num) virtual link_version_t _el_msglink_get_link_version() const noexcept override { return version_num; }

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
        void define_event(void (_LT:: *_handler)(_ET &))
        {
            std::string event_name = _ET::_event_name;

            // save to incoming and outgoing event lists
            incoming_events.insert(event_name);
            outgoing_events.insert(event_name);

            std::function<void(_LT *, _ET &)> handler = _handler;

            incoming_event_handler_map.emplace(
                _ET::_event_name,
                [this, handler](const nlohmann::json &_data) {
                std::cout << "hievent " << _data << std::endl;
                _ET new_event_inst;
                new_event_inst = _data;
                handler(
                    static_cast<_LT *>(this),
                    new_event_inst
                );
            }
            );

        }


    public:

        /**
         * @brief Construct a new link object.
         *
         * @param _is_server determines the TID series used (+n or -n)
         */
        link(bool _is_server)
            : tid_step_value(_is_server ? 1 : -1)
            , tid_counter(tid_step_value)
        {}

        /**
         * @brief valid link definitions must implement this define method
         * to define the protocol by calling the specialized define
         * methods for events and other interactions.
         *
         */
        virtual void define() noexcept = 0;

        void on_connection_established()
        {

        }

        void on_message(const std::string &_msg_content)
        {
            try
            {
                nlohmann::json jmsg = nlohmann::json::parse(_msg_content);

                // read message type and transaction ID (always present)
                std::string msg_type = jmsg.at("type");
                int transaction_id = jmsg.at("tid");

                if (authentication_done)
                {
                    handle_message_post_auth(
                        msg_type_from_string(msg_type),
                        transaction_id,
                        jmsg
                    );
                }
                else
                {
                    handle_message_pre_auth(
                        msg_type_from_string(msg_type),
                        transaction_id,
                        jmsg
                    );
                }
            }
            catch (const nlohmann::json::exception &e)
            {
                throw malformed_message_error(el::strutil::format("Malformed JSON link message: %s\n%s", _msg_content.c_str(), e.what()));
            }
        }
    };

} // namespace el::msglink
