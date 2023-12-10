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
#include <algorithm>

#include <nlohmann/json.hpp>

#include "../logging.hpp"
#include "../flags.hpp"
#include "../rtti_utils.hpp"
#include "event.hpp"
#include "errors.hpp"
#include "internal/msgtype.hpp"
#include "internal/messages.hpp"
#include "internal/types.hpp"
#include "internal/proto_version.hpp"
#include "internal/link_interface.hpp"
#include "internal/transaction.hpp"


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
        // the link interface representing the underlying communication class
        // used to send messages and manage the connection
        link_interface &interface;

        // the value to step for every new transaction ID generated.
        // This is set to either 1 or -1 in the constructor depending on wether
        // it is being used by a server or a client
        const int8_t tid_step_value;
        // running counter for transaction IDs (initialized to tid_step_value)
        tid_t tid_counter;
        // map of active transactions that take multiple back and forth messages to complete
        std::map<tid_t, transaction_ptr_t> active_transactions;

        // flags set to track the authentication process
        soflag auth_ack_sent;
        soflag auth_ack_received;
        // set as soon as the login_ack has been sent and received
        soflag authentication_done;

        // type of the lambda used to wrap event handlers
        using event_handler_wrapper_t = std::function<void(const nlohmann::json &)>;

        // set of all possible outgoing events that are defined (including bidirectional ones)
        std::set<std::string> available_outgoing_events;
        // set of all outgoing events that the other party has subscribed and therefore need to be transmitted
        std::set<std::string> active_outgoing_events;
        // set of all possible incoming events that are defined (including bidirectional ones)
        std::set<std::string> available_incoming_events;
        // map of all incoming events that we have currently subscribed to their handlers
        std::unordered_map<
            std::string,
            event_handler_wrapper_t
        > active_incoming_event_handlers;

    private:    // methods

        /**
         * @return tid_t new transaction ID according to the
         * internal running series
         */
        tid_t generate_new_tid() noexcept
        {
            // use value before counting, so first value is 1/-1
            // as defined in the spec
            auto tmp = tid_counter;
            tid_counter += tid_step_value;
            return tmp;
        }

        /**
         * @brief Creates and registers a new transaction
         * with given type, ID and init parameters in the active
         * transaction map.
         * It then returns a pointer to the created transaction
         *
         * @tparam _TR transaction type
         * @tparam _Args ctor parameter types
         * @param _tid transaction ID
         * @param _args further ctor arguments
         * @return std::shared_ptr<_TR> pointer to created transaction
         */
        template<std::derived_from<transaction_t> _TR, typename... _Args>
        inline std::shared_ptr<_TR> create_transaction(tid_t _tid, _Args ..._args)
        {
            if (active_transactions.contains(_tid))
                throw duplicate_transaction_error("Transaction with ID=%d already exists", _tid);

            auto new_transaction = std::make_shared<_TR>(
                _tid,
                std::forward<_Args>(_args)...
            );
            active_transactions[_tid] = new_transaction;

            return new_transaction;
        }

        /**
         * @brief Retrieves the active transaction of the required
         * type and ID. If there is no transaction with this ID or the
         * transaction does not match the expected type,
         * invalid_transaction_error is thrown.
         *
         * @tparam _TR expected transaction type
         * @param _tid ID of action to retrieve
         * @return std::shared_ptr<_TR> the targeted action
         */
        template<std::derived_from<transaction_t> _TR>
        inline std::shared_ptr<_TR> get_transaction(tid_t _tid)
        {
            transaction_ptr_t transaction;

            try
            {
                transaction = active_transactions.at(_tid);
            }
            catch (const std::out_of_range)
            {
                throw invalid_transaction_error("No active transaction with ID=%d", _tid);
            }

            std::shared_ptr<_TR> target_type_transaction =
                std::dynamic_pointer_cast<_TR>(transaction);

            if (target_type_transaction == nullptr)
                throw invalid_transaction_error(
                    "Active transaction with ID=%d (%s) does not match the required type %s",
                    _tid,
                    rtti::demangle_if_possible(typeid(*transaction).name()).c_str(),
                    rtti::demangle_if_possible(typeid(_TR).name()).c_str()
                );

            return target_type_transaction;
        }

        /**
         * @brief completes a transaction by removing it from the
         * map of active transactions
         *
         * @tparam _TR deduced transaction type
         * @param _transaction transaction to remove
         */
        template<std::derived_from<transaction_t> _TR>
        inline void complete_transaction(const std::shared_ptr<_TR> &_transaction) noexcept
        {
            active_transactions.erase(_transaction->id);
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
                // this transaction completes immediately, no need to register

                // check protocol version if we are the higher one
                if (proto_version::current > msg.proto_version)
                    if (!proto_version::is_compatible(msg.proto_version))
                        throw incompatible_link_error(
                            close_code_t::PROTO_VERSION_INCOMPATIBLE,
                            "Incompatible protocol versions: this=%u, other=%u",
                            proto_version::to_string(proto_version::current).c_str(),
                            proto_version::to_string(msg.proto_version).c_str()
                        );

                // check user defined link version
                if (msg.link_version != get_link_version())
                    throw incompatible_link_error(
                        close_code_t::LINK_VERSION_MISMATCH,
                        "Link versions don't match: this=%u, other=%u",
                        get_link_version(),
                        msg.link_version
                    );

                // check event list
                if (!std::includes(
                    msg.events.begin(), msg.events.end(),
                    available_incoming_events.begin(), available_incoming_events.end()
                ))
                    throw incompatible_link_error(
                        close_code_t::EVENT_REQUIREMENTS_NOT_SATISFIED,
                        "Remote party does not satisfy the event requirements (missing events)"
                    );

                // check data sources

                // check procedures

                // all good, send acknowledgement message, transaction complete
                msg_auth_ack_t response;
                response.tid = msg.tid;
                interface.send_message(response);
                auth_ack_sent.set();
            }
            break;

            case AUTH_ACK:
            {
                msg_auth_ack_t msg(_jmsg);

                auto transaction = get_transaction<transaction_auth_t>(msg.tid);
                transaction->assert_is_outgoing("Received AUTH ACK for foreign AUTH transaction");

                complete_transaction(transaction);
                auth_ack_received.set();
            }
            break;

            default:
                throw malformed_message_error("Invalid pre-auth message type: %s", msg_type_to_string(_msg_type));
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
            {
                msg_evt_sub_t msg(_jmsg);

                if (!available_outgoing_events.contains(msg.name))
                {
                    // respond with nak
                    msg_evt_sub_nak_t response;
                    response.tid = msg.tid;
                    interface.send_message(response);
                    EL_LOGW("Received EVENT_SUB message for invalid event. This is likely a library implementation error and should not happen.");
                }

                // otherwise activate (=subscribe to) the event
                active_outgoing_events.insert(msg.name);

                // respond with positive acknowledgement, transaction complete
                msg_evt_sub_ack_t response;
                response.tid = msg.tid;
                interface.send_message(response);
            }
            break;
            case EVENT_SUB_ACK:
            {
                msg_evt_sub_ack_t msg(_jmsg);

                // success, simply complete the transaction if it is valid
                auto transaction = get_transaction<transaction_event_sub_t>(msg.tid);
                transaction->assert_is_outgoing("Received EVT SUB ACK for foreign EVT SUB transaction");
                complete_transaction(transaction);
            }
                break;
            case EVENT_SUB_NAK:
            {
                msg_evt_sub_nak_t msg(_jmsg);

                // complete the transaction if it is valid
                auto transaction = get_transaction<transaction_event_sub_t>(msg.tid);
                transaction->assert_is_outgoing("Received EVT SUB NAK for foreign EVT SUB transaction");
                complete_transaction(transaction);

                // if event sub failed, ...
            }
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
                throw malformed_message_error("Invalid post-auth message type: %s", msg_type_to_string(_msg_type));
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
#define EL_MSGLINK_LINK_VERSION(version_num) virtual el::msglink::link_version_t _el_msglink_get_link_version() const noexcept override { return version_num; }

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
            available_incoming_events.insert(event_name);
            available_outgoing_events.insert(event_name);

            std::function<void(_LT *, _ET &)> handler = _handler;

            active_incoming_event_handlers.emplace(
                _ET::_event_name,
                [this, handler](const nlohmann::json &_data)
            {
                EL_LOGD("hievent %s", _data.dump().c_str());
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
         * @param _interface interface representing the communication class used to manage connection
         */
        link(bool _is_server, link_interface &_interface)
            : tid_step_value(_is_server ? 1 : -1)
            , tid_counter(tid_step_value)
            , interface(_interface)
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
            EL_LOGD("connection established called");

            auto transaction = create_transaction<transaction_auth_t>(
                generate_new_tid(),
                inout_t::OUTGOING
            );

            // send initial auth message
            msg_auth_t msg;
            msg.tid = transaction->id;
            msg.proto_version = proto_version::current;
            msg.link_version = get_link_version();
            msg.events = available_outgoing_events;
            //msg.data_sources = ...;
            //msg.procedures = ...;
            interface.send_message(msg);
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
                throw malformed_message_error(
                    "Malformed JSON link message (%s auth): %s\n%s",
                    authentication_done ? "post" : "pre",
                    _msg_content.c_str(),
                    e.what()
                );
            }
        }
    };

} // namespace el::msglink
