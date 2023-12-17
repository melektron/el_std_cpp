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
    struct event_subscription
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
} // namespace el::msglink
