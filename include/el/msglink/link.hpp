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
#include <future>

#include <nlohmann/json.hpp>

#include "../logging.hpp"
#include "../flags.hpp"
#include "../rtti_utils.hpp"
#include "event.hpp"
#include "function.hpp"
#include "errors.hpp"
#include "subscriptions.hpp"
#include "internal/msgtype.hpp"
#include "internal/messages.hpp"
#include "internal/types.hpp"
#include "internal/proto_version.hpp"
#include "internal/link_interface.hpp"
#include "internal/transaction.hpp"
#include "internal/context.hpp"


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
        // global class tree context
        ct_context &ctx;

        // the link interface representing the underlying communication class
        // used to send messages and manage the connection
        link_interface &interface;

        // the value to step for every new transaction ID generated.
        // This is set to either 1 or -1 in the constructor depending on wether
        // it is being used by a server or a client
        const int8_t tid_step_value;
        // running counter for transaction IDs (initialized to tid_step_value)
        std::atomic<tid_t> tid_counter; // uses atomic fetch and count
        // map of active transactions that take multiple back and forth messages to complete
        std::map<tid_t, transaction_ptr_t> active_transactions;
        // mutex to guard transaction map
        std::mutex mu_active_transactions;  // TODO: no longer needed with context?

        // flags set to track the authentication process
        soflag auth_ack_sent;
        soflag auth_ack_received;
        // set as soon as the login_ack has been sent and received
        soflag authentication_done;

        // flag identifying if pong messages need to be sent out
        bool pong_messages_required = false;

        // set of all possible outgoing events that are defined (including bidirectional ones)
        std::set<std::string> available_outgoing_events;
        // set of all outgoing events that the other party has subscribed to and therefore need to be transmitted
        std::set<std::string> active_outgoing_events;
        // set of all possible incoming events that are defined (including bidirectional ones)
        std::set<std::string> available_incoming_events;
        // set of all incoming events that have been subscribed to and have listeners
        std::set<std::string> active_incoming_events;

        // running counter for all sorts of subscription IDs
        std::atomic<sub_id_t> sub_id_counter = 0;   // uses atomic increment
        
        // map of all active incoming events to their subscription IDs
        std::unordered_multimap<
            std::string,
            sub_id_t
        > event_names_to_subscription_id;
        // map of subscription ID to event subscription
        std::unordered_map<
            sub_id_t, 
            std::shared_ptr<event_subscription>
        > event_subscription_ids_to_objects;
        
        // set of all possible outgoing functions that this party may want to call on the other party
        std::set<std::string> available_outgoing_functions;
        // set of incoming not required, below map is used

        // type of the intermediary handler function
        using function_handler_function_t = std::function<nlohmann::json(const nlohmann::json &)>;

        // map of all incoming function names to their handlers
        std::unordered_map<
            std::string,
            function_handler_function_t
        > available_incoming_function_names_to_functions;

    private:    // methods

        /**
         * @return tid_t new transaction ID according to the
         * internal running series
         */
        tid_t generate_new_tid() noexcept
        {
            // no lock needed here because of atomic.
            // fetch the OLD value and then add step value atomically
            // so first value is 1/-1 as defined in the spec
            return tid_counter.fetch_add(tid_step_value);
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
            std::lock_guard lock(mu_active_transactions);

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
            std::lock_guard lock(mu_active_transactions);
            
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
            std::lock_guard lock(mu_active_transactions);
            
            active_transactions.erase(_transaction->id);
        }

        /**
         * @brief updates authentication done flag from
         * the other authentication flags' values
         */
        void update_auth_done() noexcept
        {
            if (auth_ack_sent && auth_ack_received && !authentication_done)
            {
                authentication_done.set();
                on_authentication_done();
            }
        }

        /**
         * @return link_version_t link version of the link instance
         */
        link_version_t get_link_version() const noexcept
        {
            return _el_msglink_get_link_version();
        }

        /**
         * @brief sends a pong message
         */
        void send_pong_message()
        {
            msg_pong_t msg;
            interface.send_message(msg);
        }

        /**
         * @brief sends an event subscribe message for a specific event
         * 
         * @param _event_name event to send sub message for
         */
        void send_event_subscribe_message(const std::string &_event_name)
        {
            msg_evt_sub_t msg;
            msg.tid = generate_new_tid();
            msg.name = _event_name;
            interface.send_message(msg);
        }

        /**
         * @brief sends an event unsubscribe message for a specific event
         * 
         * @param _event_name event to send unsub message for
         */
        void send_event_unsubscribe_message(const std::string &_event_name)
        {
            msg_evt_unsub_t msg;
            msg.tid = generate_new_tid();
            msg.name = _event_name;
            interface.send_message(msg);
        }

        /**
         * @brief encodes event data and sens an event emit message for a
         * specific event
         * 
         * @param _event_name event to send
         * @param _evt data to encode
         */
        void send_event_emit_message(
            const std::string &_event_name, 
            const outgoing_event &_evt
        ) {
            msg_evt_emit_t msg;
            msg.tid = generate_new_tid();
            msg.name = _event_name;
            msg.data = _evt;
            interface.send_message(msg);
        }

        /**
         * @brief handles incoming messages (already parsed) before authentication is complete
         * to perform the authentication.
         *
         * @param _jmsg parsed json message
         */
        void handle_message_pre_auth(
            const msg_type_t _msg_type,
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

                // check if pong messages are required
                if (msg.no_ping.has_value())
                    pong_messages_required = *msg.no_ping;
                
                // check event list
                // all the events I may require (incoming) must be included in the events
                // the other party can provide (it's outgoing events)
                if (!std::includes(
                    msg.events.begin(), msg.events.end(),
                    available_incoming_events.begin(), available_incoming_events.end()
                ))
                    throw incompatible_link_error(
                        close_code_t::EVENT_REQUIREMENTS_NOT_SATISFIED,
                        "Remote party does not satisfy the event requirements (missing events)"
                    );
                
                // check data sources

                // check functions
                // all the functions this party may call (outgoing) must be included in the other
                // parties callable (incoming) functions
                if (!std::includes(
                    msg.functions.begin(), msg.functions.end(),
                    available_outgoing_functions.begin(), available_outgoing_functions.end()
                ))
                    throw incompatible_link_error(
                        close_code_t::FUNCTION_REQUIREMENTS_NOT_SATISFIED,
                        "Remote party does not satisfy the function requirements (missing functions)"
                    );

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
                throw protocol_error("Invalid pre-auth message type: %s", msg_type_to_string(_msg_type));
                break;
            }

            update_auth_done();
        }
        
        /**
         * @brief called immediately after authentication done flag is set
         * (as soon as both parties are authenticated).
         * This function sends some initial post-auth messages to the other 
         * party.
         */
        void on_authentication_done()
        {
            EL_LOG_FUNCTION_CALL();

            // send event subscribe messages for all events subscribed before 
            // auth was complete (e.g. events with fixed handlers created during
            // definition)
            for (const auto &event_name : active_incoming_events)
            {
                send_event_subscribe_message(event_name);
            }

            // ... do same for datasubs and RPCs
        }

        /**
         * @brief handles incoming messages (already parsed) after authentication is complete
         * and both parties are authenticated.
         *
         * @param _jmsg parsed message
         */
        void handle_message_post_auth(
            const msg_type_t _msg_type,
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
                    EL_LOGW("Received EVENT_SUB message for invalid event. This is likely a library implementation issue and should not happen.");
                    break;
                }

                // otherwise activate (=subscribe to) the event
                active_outgoing_events.insert(msg.name);
                
                // no response required
            }
            break;
            case EVENT_UNSUB:
            {
                msg_evt_unsub_t msg(_jmsg);

                if (!active_outgoing_events.contains(msg.name))
                {
                    EL_LOGW("Received EVENT_UNSUB message for an event which was not subscribed and/or doesn't exist. This is likely a library implementation issue and should not happen.");
                    break;
                }

                // otherwise unsubscribe from the event
                active_outgoing_events.erase(msg.name);
                
                // no response required
            }
            break;
            case EVENT_EMIT:
            {
                msg_evt_emit_t msg(_jmsg);

                if (!active_incoming_events.contains(msg.name) || !event_names_to_subscription_id.contains(msg.name))
                {
                    EL_LOGW("Received EVENT_EMIT message for an event which was not subscribed to, isn't incoming and/or doesn't exist. This is likely a library implementation issue and should not happen.");
                    break;
                }

                // call all the listeners
                auto range = event_names_to_subscription_id.equal_range(msg.name);  // this doesn't throw even when there are no matches
                for (auto it = range.first; it != range.second; ++it)
                {
                    try
                    {
                        auto sub = event_subscription_ids_to_objects.at(it->second);
                        // TODO: possibly release lock here temporarily as control is passed to user code?
                        // TODO: or maybe make this an async asio call?
                        // TODO: if lock is released, shouldn't it be done inside the subscription?
                        sub->call_handler(msg.data);
                    }
                    catch(const std::out_of_range& e)
                    {
                        throw invalid_identifier_error("Attempted to call event listener of invalid subscription ID. This is likely due to a library bug.");
                    }
                }

                // no response required
            }
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
            case FUNC_CALL:
            {
                msg_func_call_t msg(_jmsg);

                if (!available_incoming_function_names_to_functions.contains(msg.name))
                {
                    EL_LOGW("Received FUNC_CALL message for a function which isn't incoming and/or doesn't exist. This is likely a library implementation issue and should not happen.");
                    break;
                }

                // run the handler
                nlohmann::json results_object;
                try
                {
                    // TODO: possibly release lock here temporarily as control is passed to user code?
                    // TODO: or maybe make this an async asio call?
                    results_object = available_incoming_function_names_to_functions.at(msg.name)(msg.params);
                }
                catch (const std::exception &_e)
                {
                    // error during handler execution, respond with error message
                    msg_func_err_t response;
                    response.tid = msg.tid;
                    response.info = _e.what();
                    interface.send_message(response);
                    break;
                }

                // otherwise send result
                msg_func_result_t response;
                response.tid = msg.tid;
                response.results = results_object;
                interface.send_message(response);
            }
            break;
            case FUNC_ERR:
            {
                msg_func_err_t msg(_jmsg);
                auto transaction = get_transaction<transaction_function_call_t>(msg.tid);
                
                // run error callback to complete future
                if (transaction->handle_error != nullptr)
                    transaction->handle_error(msg.info);

                // complete transaction -> destroys lambdas and releases promise
                complete_transaction(transaction);
            }
            break;
            case FUNC_RESULT:
            {
                msg_func_result_t msg(_jmsg);
                auto transaction = get_transaction<transaction_function_call_t>(msg.tid);
                
                // run result callback to complete future
                if (transaction->handle_result != nullptr)
                    transaction->handle_result(msg.results);

                // complete transaction -> destroys lambdas and releases promise
                complete_transaction(transaction);
            }
            break;

            default:
                throw protocol_error("Invalid post-auth message type: %s", msg_type_to_string(_msg_type));
                break;
            }

        }

        /**
         * @return sub_id_t new unique subscription ID
         */
        sub_id_t generate_new_sub_id() noexcept
        {
            return ++sub_id_counter;
        }

        /**
         * @brief registers an event subscription in the internal
         * map and subscribes the event from the other party
         * if it isn't already.
         */
        subscription_hdl_ptr<event_subscription> add_event_subscription(
            const std::string &_event_name,
            event_subscription::handler_function_t _handler_function
        ) {

            std::string event_name = _event_name;   // copy for lambda capture
            // create subscription object
            const sub_id_t sub_id = generate_new_sub_id();
            auto subscription = std::shared_ptr<event_subscription>(new event_subscription(
                _handler_function,
                [this, _event_name, sub_id](void)   // cancel function
                {
                    EL_LOGD("cancel event %s:%d", _event_name.c_str(), sub_id);
                    // create copy of name and ID because this lambda and it's captures 
                    // may be destroyed during the below function call
                    std::string l_event_name = _event_name;
                    sub_id_t l_sub_id = sub_id;
                    this->remove_event_subscription(l_event_name, l_sub_id);
                }
            ));

            // register the subscription
            event_subscription_ids_to_objects.emplace(
                sub_id,
                subscription
            );
            event_names_to_subscription_id.emplace(
                _event_name,
                sub_id
            );

            // activate the event if it is not already active
            if (active_incoming_events.contains(_event_name))
                goto exit; // another listener already exits, the event is already active
            
            // add to list of active events
            active_incoming_events.insert(_event_name);
            // if authentication is done already send the subscribe message now. 
            // If auth is not done, sub messages will be sent as soon
            // as authentication_done is set.
            if (authentication_done)
                send_event_subscribe_message(_event_name);

        exit:
            return subscription_hdl_ptr<event_subscription>(
                new subscription_hdl<event_subscription>(
                    subscription
                )
            );
        }

        /**
         * @brief removes an event subscription and deactivates
         * the event by sending unsubscribe message if required
         * 
         * @param _event_name 
         * @param _subscription_id 
         */
        void remove_event_subscription(
            const std::string &_event_name,
            sub_id_t _subscription_id
        ) {
            // count amount of subscriptions left
            size_t sub_count = 0;

            // iterator to store position to delete
            auto target_it = event_names_to_subscription_id.end();

            // go through all subscriptions, counting them and identifying the ony that is to be deleted
            auto range = event_names_to_subscription_id.equal_range(_event_name);  // this doesn't throw even when there are no matches
            for (auto it = range.first; it != range.second; ++it)
            {
                sub_count++;

                auto sub_id = it->second;
                if (it->second == _subscription_id)
                {
                    // save position
                    target_it = it;
                    // subscription cannot be erased directly here because iterators would be invalidated
                }
            }
            
            // if a subscription was found, delete it now and remove it from count
            if (target_it != event_names_to_subscription_id.end())
            {
                event_names_to_subscription_id.erase(target_it);
                sub_count--;
            }

            // if there are no subscriptions left, deactivate the event
            if (sub_count == 0)
            {
                active_incoming_events.erase(_event_name);
                if (authentication_done)
                    send_event_unsubscribe_message(_event_name);
            }

            // remove from id to sub object set
            // Attention: This might cause the object to be destroyed, which
            // will cause any parameters passed to this function by reference
            // from a lambda context to become dangling pointers. After this, don't 
            // use parameters anymore if possible even though our cancel() lambda is designed
            // in a way to avoid this issue by copying values to stack.
            try
            {
                // make sure the subscription is invalidated 
                event_subscription_ids_to_objects.at(_subscription_id)->invalidate();
                // possibly delete the element
                event_subscription_ids_to_objects.erase(_subscription_id);
            }
            catch(const std::out_of_range& e)
            {
                throw invalid_identifier_error("Attempted to remove event subscription with invalid subscription ID %d. This is likely a library bug.", _subscription_id);
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
         * The following methods are used to define events, data subscriptions
         * and RPCs and provide optional shortcut functionality
         */

        /**
         * @brief Defines a bidirectional event. This method does not add
         * any listeners.
         * 
         * @tparam _ET the event class of the event to register 
         *             (must inherit from el::msglink::incoming_event and el::msglink::outgoing_event 
         *              (aka. el::msglink::bidirectional_event))
         */
        template <BidirectionalEvent _ET>
        void define_event()
        {
            // save name
            std::string event_name = _ET::_event_name;

            // define as incoming and outgoing
            available_incoming_events.insert(event_name);
            available_outgoing_events.insert(event_name);
        }

        /**
         * @brief Shortcut for defining a bidirectional event
         * and adding an event listener that is a method of the link.
         * 
         * The event listener must be a method
         * of the link it is registered on. This is a shortcut
         * to avoid having to use std::bind to bind listener
         * to the instance. When an external listener function is needed, this
         * is the wrong overload.
         *
         * @note Method function pointer:
         * https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
         *
         * @tparam _ET the event class of the event to register
         *             (must inherit from el::msglink::incoming_event and el::msglink::outgoing_event 
         *              (aka. el::msglink::bidirectional_event), can be deduced from method parameter)
         * @tparam _LT the link class the handler is a method of (can also be deduced)
         * @param _listener the handler method for the event
         */
        template <BidirectionalEvent _ET, std::derived_from<link> _LT>
        event_sub_hdl_ptr define_event(
            void (_LT:: *_listener)(_ET &)
        ) {
            // save name and handler function
            std::string event_name = _ET::_event_name;
            std::function<void(_LT *, _ET &)> listener = _listener;

            // define as incoming and outgoing
            available_incoming_events.insert(event_name);
            available_outgoing_events.insert(event_name);

            // create subscription with handler function
            return add_event_subscription(
                event_name,
                [this, listener](const nlohmann::json &_data)
                {
                    EL_LOGD("hievent %s", _data.dump().c_str());
                    _ET new_event_inst;
                    new_event_inst = _data;
                    listener(
                        static_cast<_LT *>(this),
                        new_event_inst
                    );
                }
            );
        }

        /**
         * @brief Shortcut for defining a bidirectional event
         * and adding an event listener that is an arbitrary function.
         * 
         * The event listener can be an arbitrary function matching the call signature
         * ```
         * void(_ET &_evt)
         * ```.
         * If the listener is a method of the link instance,
         * there is a special overload to simplify that case. This is not that overload.
         *
         * @tparam _ET the event class of the event to register
         *             (must inherit from el::msglink::incoming_event and el::msglink::outgoing_event 
         *              (aka. el::msglink::bidirectional_event), can be deduced from method parameter)
         * @param _listener the handler function for the event
         */
        template <BidirectionalEvent _ET>
        event_sub_hdl_ptr define_event(
            void (*_listener)(_ET &)
        ) {
            // save name and handler function
            std::string event_name = _ET::_event_name;
            std::function<void(_ET &)> listener = _listener;

            // define as incoming and outgoing
            available_incoming_events.insert(event_name);
            available_outgoing_events.insert(event_name);

            // create subscription with handler function
            return add_event_subscription(
                event_name,
                [this, listener](const nlohmann::json &_data)
                {
                    EL_LOGD("hievent %s", _data.dump().c_str());
                    _ET new_event_inst;
                    new_event_inst = _data;
                    listener(
                        new_event_inst
                    );
                }
            );
        }

        /**
         * @brief Defines an incoming only event. This method does not add
         * any listeners.
         * 
         * @tparam _ET the event class of the event to register (must inherit from el::msglink::incoming_event)
         */
        template <IncomingOnlyEvent _ET>
        void define_event()
        {
            // save name
            std::string event_name = _ET::_event_name;

            // define as incoming
            available_incoming_events.insert(event_name);
        }

        /**
         * @brief Shortcut for defining an incoming only event
         * and adding an event listener that is a method of the link.
         * 
         * The event listener must be a method
         * of the link it is registered on. This is a shortcut
         * to avoid having to use std::bind to bind listener
         * to the instance. When an external listener function is needed, this
         * is the wrong overload.
         *
         * @note Method function pointer:
         * https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
         *
         * @tparam _ET the event class of the event to register
         *             (must inherit from el::msglink::incoming_event, can be deduced from method parameter)
         * @tparam _LT the link class the handler is a method of (can also be deduced)
         * @param _listener the handler method for the event
         */
        template <IncomingOnlyEvent _ET, std::derived_from<link> _LT>
        event_sub_hdl_ptr define_event(
            void (_LT:: *_listener)(_ET &)
        ) {
            // save name and handler function
            std::string event_name = _ET::_event_name;
            std::function<void(_LT *, _ET &)> listener = _listener;

            // define as incoming
            available_incoming_events.insert(event_name);

            // create subscription with handler function
            return add_event_subscription(
                event_name,
                [this, listener](const nlohmann::json &_data)
                {
                    EL_LOGD("hievent %s", _data.dump().c_str());
                    _ET new_event_inst;
                    new_event_inst = _data;
                    listener(
                        static_cast<_LT *>(this),
                        new_event_inst
                    );
                }
            );
        }

        /**
         * @brief Shortcut for defining an incoming only event
         * and adding an event listener that is an arbitrary function.
         * 
         * The event listener can be an arbitrary function matching the call signature
         * ```
         * void(_ET &_evt)
         * ```.
         * If the listener is a method of the link instance,
         * there is a special overload to simplify that case. This is not that overload.
         *
         * @tparam _ET the event class of the event to register
         *             (must inherit from el::msglink::incoming_event, can be deduced from method parameter)
         * @param _listener the handler function for the event
         */
        template <IncomingOnlyEvent _ET>
        event_sub_hdl_ptr define_event(
            void (*_listener)(_ET &)
        ) {
            // save name and handler function
            std::string event_name = _ET::_event_name;
            std::function<void(_ET &)> listener = _listener;

            // define as incoming
            available_incoming_events.insert(event_name);

            // create subscription with handler function
            return add_event_subscription(
                event_name,
                [this, listener](const nlohmann::json &_data)
                {
                    EL_LOGD("hievent %s", _data.dump().c_str());
                    _ET new_event_inst;
                    new_event_inst = _data;
                    listener(
                        new_event_inst
                    );
                }
            );
        }

        /**
         * @brief Defines an outgoing only event.
         * 
         * @tparam _ET the event class of the event to register (must inherit from el::msglink::outgoing_event)
         */
        template <OutgoingOnlyEvent _ET>
        void define_event()
        {
            // save name
            std::string event_name = _ET::_event_name;

            // define as outgoing
            available_outgoing_events.insert(event_name);
        }


        /**
         * == Functions ==
         * 
         */
        
        /**
         * @brief Shortcut for defining a bidirectional function
         * with a function that is a method of the link.
         * 
         * The function must be a method
         * of the link it is registered on. This is a shortcut
         * to avoid having to use std::bind to bind the function
         * to the instance. When an external function is needed, this
         * is the wrong overload.
         *
         * @note Method function pointer:
         * https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
         *
         * @tparam _FT the function class of the function to register
         *             (must inherit from el::msglink::incoming_function and el::msglink::outgoing_function, can be deduced from method parameter)
         * @tparam _LT the link class the handler function is a method of (can also be deduced)
         * @param _handler the method containing the function code
         */
        template <BidirectionalFunction _FT, std::derived_from<link> _LT>
        void define_function(
            typename _FT::results_t (_LT:: *_handler)(typename _FT::parameters_t &)
        ) {
            // save name and handler function
            std::string function_name = _FT::_function_name;
            std::function<typename _FT::results_t (_LT *, typename _FT::parameters_t &)> handler_fn = _handler;

            // define outgoing
            available_outgoing_functions.insert(function_name);

            // define as incoming by creating intermediary handler for data conversion
            available_incoming_function_names_to_functions[function_name] = [this, handler_fn](const nlohmann::json &_data) -> nlohmann::json
            {
                EL_LOGD("proc hdl %s", _data.dump().c_str());
                typename _FT::parameters_t function_parameters = _data;
                return static_cast<nlohmann::json>(handler_fn(
                    static_cast<_LT *>(this),
                    function_parameters
                ));
            };
        }

        /**
         * @brief Shortcut for defining a bidirectional function
         * with an arbitrary handler function.
         * 
         * The handler can be an arbitrary function matching the call signature
         * ```
         * _FT::results_t(_FT::parameters_t &_params)
         * ```.
         * If the handler is a method of the link instance,
         * there is a special overload to simplify that case. This is not that overload.
         *
         * @tparam _FT the function class of the function to register
         *             (must inherit from el::msglink::incoming_function and el::msglink::outgoing_function, can be deduced from method parameter)
         * @param _handler the method containing the function code
         */
        template <BidirectionalFunction _FT>
        void define_function(
            typename _FT::results_t (*_handler)(typename _FT::parameters_t &)
        ) {
            // save name and handler function
            std::string function_name = _FT::_function_name;
            std::function<typename _FT::results_t (typename _FT::parameters_t &)> handler_fn = _handler;

            // define as outgoing
            available_outgoing_functions.insert(function_name);

            // define as incoming by creating intermediary handler for data conversion
            available_incoming_function_names_to_functions[function_name] = [this, handler_fn](const nlohmann::json &_data) -> nlohmann::json
            {
                EL_LOGD("proc hdl %s", _data.dump().c_str());
                typename _FT::parameters_t function_parameters = _data;
                return static_cast<nlohmann::json>(handler_fn(
                    function_parameters
                ));
            };
        }

        /**
         * @brief Shortcut for defining an incoming only function
         * with a function that is a method of the link.
         * 
         * The function must be a method
         * of the link it is registered on. This is a shortcut
         * to avoid having to use std::bind to bind the function
         * to the instance. When an external function is needed, this
         * is the wrong overload.
         *
         * @note Method function pointer:
         * https://isocpp.org/wiki/faq/pointers-to-members#typedef-for-ptr-to-memfn
         *
         * @tparam _FT the function class of the function to register
         *             (must inherit from el::msglink::incoming_function, can be deduced from method parameter)
         * @tparam _LT the link class the handler function is a method of (can also be deduced)
         * @param _handler the method containing the function code
         */
        template <IncomingOnlyFunction _FT, std::derived_from<link> _LT>
        void define_function(
            typename _FT::results_t (_LT:: *_handler)(typename _FT::parameters_t &)
        ) {
            // save name and handler function
            std::string function_name = _FT::_function_name;
            std::function<typename _FT::results_t (_LT *, typename _FT::parameters_t &)> handler_fn = _handler;

            // define as incoming by creating intermediary handler for data conversion
            available_incoming_function_names_to_functions[function_name] = [this, handler_fn](const nlohmann::json &_data) -> nlohmann::json
            {
                EL_LOGD("proc hdl %s", _data.dump().c_str());
                typename _FT::parameters_t function_parameters = _data;
                return static_cast<nlohmann::json>(handler_fn(
                    static_cast<_LT *>(this),
                    function_parameters
                ));
            };
        }

        /**
         * @brief Shortcut for defining an incoming only function
         * with an arbitrary handler function.
         * 
         * The handler can be an arbitrary function matching the call signature
         * ```
         * _FT::results_t(_FT::parameters_t &_params)
         * ```.
         * If the handler is a method of the link instance,
         * there is a special overload to simplify that case. This is not that overload.
         *
         * @tparam _FT the function class of the function to register
         *             (must inherit from el::msglink::incoming_function, can be deduced from method parameter)
         * @param _handler the method containing the function code
         */
        template <IncomingOnlyFunction _FT>
        void define_function(
            typename _FT::results_t (*_handler)(typename _FT::parameters_t &)
        ) {
            // save name and handler function
            std::string function_name = _FT::_function_name;
            std::function<typename _FT::results_t (typename _FT::parameters_t &)> handler_fn = _handler;

            // define as incoming by creating intermediary handler for data conversion
            available_incoming_function_names_to_functions[function_name] = [this, handler_fn](const nlohmann::json &_data) -> nlohmann::json
            {
                EL_LOGD("proc hdl %s", _data.dump().c_str());
                typename _FT::parameters_t function_parameters = _data;
                return static_cast<nlohmann::json>(handler_fn(
                    function_parameters
                ));
            };
        }

        /**
         * @brief Defines an outgoing only function.
         * 
         * @tparam _FT the function class of the function to register (must inherit from el::msglink::outgoing_function)
         */
        template <OutgoingOnlyFunction _FT>
        void define_function()
        {
            // save name
            std::string function_name = _FT::_function_name;

            // define as outgoing
            available_outgoing_functions.insert(function_name);
        }

    public:

        /**
         * The following functions are used to access events, data subscriptions 
         * or RPCs such as by registering listeners, emitting events or updating data.
         */

        /**
         * @brief emits a msglink event.
         * 
         * @attention (external entry: public method)
         * @tparam _ET event type to emit
         * @param _event event body to emit
         */
        template<AtLeastOutgoingEvent _ET>
        void emit(const _ET &_event)
        {
            auto lock = ctx.get_lock();
            
            // make sure that this event is defined
            if (!available_outgoing_events.contains(_ET::_event_name))
                throw invalid_outgoing_event_error("Event '%s' cannot be emitted because it is not defined as outgoing", _ET::_event_name);

            // check if the event is needed
            if (!active_outgoing_events.contains(_ET::_event_name))
                return;

            send_event_emit_message(_ET::_event_name, _event);
        }
        
        /**
         * @brief calls (or rather initiates) a msglink remote function.
         * This returns a future that will contain the result of the function as soon
         * as the remote party as responded with the result, or an exception if it 
         * responds with an error. This "call" method is supposed to be called from an 
         * external thread that is also able to await the future. Awaiting the future on the
         * communication thread will block the asio i/o loop and therefore result in a deadlock.
         * 
         * @attention (external entry: public method)
         * @tparam _FT type of the function to call
         * @param _params function parameters to pass
         * @return std::future<typename _FT::results_t> future function results data
         */
        template<AtLeastOutgoingFunction _FT>
        std::future<typename _FT::results_t> call(const typename _FT::parameters_t &_params) 
        {
            auto lock = ctx.get_lock();
            
            // create the transaction (this this initializes the promise)
            auto transaction = create_transaction<transaction_function_call_t>(
                generate_new_tid(),
                inout_t::OUTGOING
            );

            // create promise for result (shared, will be deleted when lambdas are released)
            auto promise = std::make_shared<std::promise<typename _FT::results_t>>();

            // register response handlers
            // (being careful not to introduce cyclic references via shared ptr)
            transaction->handle_result = [promise](     // called from withing message handler, no external entry
                const nlohmann::json &_result
            ) {
                try
                {   
                    // decode results from json and return them
                    promise->set_value(_result);
                }
                catch (const std::exception &)
                {
                    // set exception if decode fails
                    promise->set_exception(std::current_exception());
                }
            };
            transaction->handle_error = [promise](      // called from withing message handler, no external entry
                const std::string &_info
            ) {
                // save error in promise
                promise->set_exception(
                    std::make_exception_ptr(
                        remote_function_error(_info)
                    )
                );
            };

            // send call message
            msg_func_call_t msg;
            msg.tid = transaction->id;
            msg.name = _FT::_function_name;
            msg.params = _params;
            interface.send_message(msg);

            // return future
            return promise->get_future();
        }

    public:

        /**
         * @brief Construct a new link object.
         *
         * @param _ctx global class tree context passed from the owning class
         * @param _is_server determines the TID series used (+n or -n)
         * @param _interface interface representing the communication class used to manage connection
         */
        link(ct_context &_ctx, bool _is_server, link_interface &_interface)
            : ctx(_ctx)
            , tid_step_value(_is_server ? 1 : -1)
            , tid_counter(tid_step_value)
            , interface(_interface)
        {}

        ~link()
        {
            EL_LOG_FUNCTION_CALL();
            // invalidate all event subscriptions to make sure there are no dangling pointers
            for (auto &[id, sub] : event_subscription_ids_to_objects)
                sub->invalidate();
        }

        /**
         * @brief valid link definitions must implement this define method
         * to define the protocol by calling the specialized define
         * methods for events and other interactions.
         *
         */
        virtual void define() noexcept = 0;

        /**
         * @brief called by link interface when the connection has been 
         * established and communication can begin.
         */
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
            msg.events = available_outgoing_events; // all events this party can provide (so the other one can subscribe to them)
            //msg.data_sources = ...;
            // all functions this party can provide (so the other one can call them)
            std::transform( // using transform to fill msg.functions with key from function map
                available_incoming_function_names_to_functions.begin(), available_incoming_function_names_to_functions.end(),
                std::inserter(msg.functions, msg.functions.end()),
                [](auto entry){ return entry.first; }
            );
            interface.send_message(msg);
        }

        /**
         * @brief called by link interface when an incoming message has been received.
         * 
         * @param _msg_content message data
         */
        void on_message(const std::string &_msg_content)
        {
            try
            {
                nlohmann::json jmsg = nlohmann::json::parse(_msg_content);

                // read message type (always present)
                std::string msg_type = jmsg.at("type");
                // if we received a pong message, ignore it but give warning.
                // This should never happen as the msglink C++ client doesn't need request this.
                // And in case we are a server, we shouldn't be getting it in the first place
                if (msg_type == __EL_MSGLINK_MSG_NAME_PONG)
                {
                    EL_LOGW("Received msglink PONG message even though msglink C++ client's don't require it and/or we are a server.");
                    return;
                }

                if (authentication_done)
                {
                    handle_message_post_auth(
                        msg_type_from_string(msg_type),
                        jmsg
                    );
                }
                else
                {
                    handle_message_pre_auth(
                        msg_type_from_string(msg_type),
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

        /**
         * @brief called by link interface of server when WS pong has been 
         * received.
         * Causes pong message to be transmitted if required.
         */
        void on_pong_received()
        {
            // no lock required here because this doesn't access any user-defined data

            if (pong_messages_required)
                send_pong_message();
        }

    };

} // namespace el::msglink
