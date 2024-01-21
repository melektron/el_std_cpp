/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
17.12.23, 21:10
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Structures representing various types of subscriptions.
*/

#pragma once

#include <functional>
#include <string>
#include <nlohmann/json.hpp>

#include "../flags.hpp"
#include "../logging.hpp"
#include "internal/types.hpp"


namespace el::msglink
{
    /**
     * @brief structure representing an event subscription which
     * is used to identify and cancel the subscription later.
     * 
     */
    class event_subscription
    {
    protected:
        friend class link;

        // invalidates all potential references and callbacks to the link to 
        // prevent any calls back to a potentially deallocated link instance. 
        // This is called by the link destructor.
        void invalidate()
        {
            cancel_function = nullptr;
            handler_function = nullptr;
        }

        // type of the lambda used to wrap event handlers
        using handler_function_t = std::function<void(const nlohmann::json &)>;
        // type of the lambda used for the cancel callback
        using cancel_function_t = std::function<void()>;

        // function called when the event is received
        handler_function_t handler_function;
        // function called to cancel the event (will be a
        // lambda created by the link)
        cancel_function_t cancel_function;

        void call_handler(const nlohmann::json &_data)
        {   
            if (handler_function != nullptr)
                handler_function(_data);
        }

        event_subscription(
            handler_function_t _handler_function,
            cancel_function_t _cancel_function
        )
            : handler_function(_handler_function)
            , cancel_function(_cancel_function)
        {}

    public:
        event_subscription(const event_subscription &) = default;
        event_subscription(event_subscription &&) = default;

        /**
         * @brief function to cancel the subscription and therefore
         * unsubscribe from the event. If already canceled, this does 
         * nothing.
         */
        void cancel()
        {
            if (cancel_function != nullptr)
            {
                cancel_function();
                cancel_function = nullptr;  // only cancel once
            }
        }

        /**
         * @brief Invalidates the object
         */
        ~event_subscription()
        {
            EL_LOG_FUNCTION_CALL();
            invalidate();
        }
    };
    
    /**
     * @brief a class which handles subscription lifetime using 
     * RAII functionality.
     * When a subscription is returned from the msglink library
     * to user code, it is wrapped by a subscription handle. The handle
     * itself is wrapped by a shared_ptr, so there is only one handle
     * per subscription.
     * The subscription is automatically canceled when the lifetime of 
     * the subscription handle ends, i.e. when no nobody holds a reference
     * to it anymore.
     * One can also cancel the subscription manually before this happens using
     * the cancel() method.
     * 
     * This is to prevent callbacks to class instances which don't exist anymore 
     * when forgetting to cancel subscriptions in class destructors.
     */
    template<typename _SUB_T>
    class subscription_hdl
    {
        friend class link;

    protected:

        // the managed subscription
        std::shared_ptr<_SUB_T> subscription_ptr;

        // no copy or move
        subscription_hdl(const subscription_hdl &) = delete;
        subscription_hdl(subscription_hdl &&) = delete;
        // only link is allowed to construct instance with valid subscription pointer
        subscription_hdl(std::shared_ptr<_SUB_T> _sub_ptr)
            : subscription_ptr(_sub_ptr)
        {
            EL_LOG_FUNCTION_CALL();}

    public:
        ~subscription_hdl()
        {
            EL_LOG_FUNCTION_CALL();
            if (subscription_ptr != nullptr)
                subscription_ptr->cancel();
        }

        /**
         * @brief cancels the subscription even if there
         * are still references to the subscription_hdl.
         * Usually, this is not required.
         */
        void cancel()
        {
            if (subscription_ptr != nullptr)
                subscription_ptr->cancel();
        }
    };
    
    // shortcut for shared pointer to subscription handle
    template<class _SUB_T>
    using subscription_hdl_ptr = std::shared_ptr<subscription_hdl<_SUB_T>>;

    /**
     * @brief shortcut for shared_ptr to subscription_hdl for event_subscription.
     * This type is library user-facing, so this is a short alias.
     */
    using event_sub_hdl_ptr = subscription_hdl_ptr<event_subscription>;
} // namespace el::msglink
