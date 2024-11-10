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
#include "internal/context.hpp"


namespace el::msglink
{
    /**
     * @brief structure representing an event subscription which
     * is used to identify and cancel the subscription later.
     * This is not a public class. It is only for use withing the communication
     * class tree.
     */
    class event_subscription
    {
    protected:
        friend class link;

        // type of the lambda used to wrap event handlers
        using handler_function_t = std::function<void(const nlohmann::json &)>;
        // type of the lambda used for the cancel callback
        using cancel_function_t = std::function<void()>;

        // global class tree context
        ct_context &ctx;

        // function called when the event is received
        handler_function_t handler_function;
        // function called to cancel the event (will be a
        // lambda created by the link)
        cancel_function_t cancel_function;

        // invalidates all potential references and callbacks to the link to 
        // prevent any calls back to a potentially deallocated link instance. 
        // This is called by the link destructor, so it is no external entry
        void invalidate()
        {
            cancel_function = nullptr;
            handler_function = nullptr;
        }

        /**
         * @brief Function to be called from an asio callback
         * when the event is received. Because this is called by
         * an asio callback, this is an external entry.
         * 
         * @attention (external entry: asio callback)
         * @param _data 
         */
        void asio_cb_call_handler(const nlohmann::json &_data)
        {   
            auto lock = ctx.get_lock();

            if (handler_function != nullptr)
            {
                auto handler_copy = handler_function;
                // Unlock to allow user callback to run without lock, so it can call external tree entries
                lock.unlock();
                // possible inconsistency problem when subscription is canceled by another thread right here.
                // The function will still be called because of the copy, but user may not expect it.
                // At this time, I don't know how to solve this without keeping the tree locked.
                handler_copy(_data);
                lock.lock();    // re-lock after callback in case more has to be done
            }
        }

        /**
         * @brief Construct a new subscription
         * 
         * @param _ctx global class tree context
         * @param _handler_function user code handler function (with decode wrapper)
         * @param _cancel_function internal cancellation function
         */
        event_subscription(
            ct_context &_ctx,
            handler_function_t _handler_function,
            cancel_function_t _cancel_function
        )
            : ctx(_ctx)
            , handler_function(_handler_function)
            , cancel_function(_cancel_function)
        {}

    public:
        event_subscription(const event_subscription &) = default;
        event_subscription(event_subscription &&) = default;

        /**
         * @brief Invalidates the object.
         */
        ~event_subscription()
        {
            auto lock = ctx.get_soft_lock();

            EL_LOG_FUNCTION_CALL();
            
            invalidate();
        }

        /**
         * @brief function to cancel the subscription and therefore
         * unsubscribe from the event. If already canceled, this does 
         * nothing.
         * 
         * This is only called from subscription handle and is therefore
         * not an external entry.
         */
        void cancel()
        {
            if (cancel_function != nullptr)
            {
                cancel_function();
                cancel_function = nullptr;  // only cancel once
            }
        }
    };
    
    /**
     * @brief a class which handles subscription lifetime using 
     * RAII functionality.
     * When a subscription is returned from the msglink library
     * to user code, it is wrapped by a subscription handle. The handle
     * itself is wrapped by a shared_ptr, so there is only ever one handle
     * per subscription.
     * The subscription is automatically canceled when the lifetime of 
     * the subscription handle ends, i.e. when no nobody holds a reference
     * to it anymore.
     * This is to prevent callbacks to class instances which don't exist anymore 
     * when forgetting to cancel subscriptions in class destructors.
     * One can also cancel the subscription manually before this happens using
     * the cancel() method though.
     * 
     * @attention The subscription handle is an entirely external, public object. Apart
     * from construction, it is never accessed from the communication class tree.
     * As such, all functions it calls internally count as external entries.
     */
    template<typename _SUB_T>
    class subscription_hdl
    {
        friend class link;

    protected:

        // global class tree context
        ct_context &ctx;

        // the managed subscription (this instance and the link instance are the only two references of this)
        std::shared_ptr<_SUB_T> subscription_ptr;   // must be a counted reference (not weak) to ensure ->cancel() is called on subscription 

        // no copy or move
        subscription_hdl(const subscription_hdl &) = delete;
        subscription_hdl(subscription_hdl &&) = delete;

        // only link is allowed to construct instance with valid subscription pointer
        subscription_hdl(
            ct_context &_ctx,
            std::shared_ptr<_SUB_T> _sub_ptr
        )
            : ctx(_ctx)
            , subscription_ptr(_sub_ptr)
        {
            EL_LOG_FUNCTION_CALL();
        }

    public:

        /**
         * cancels the subscription
         */
        ~subscription_hdl()
        {
            // possibly external entry
            auto lock = ctx.get_soft_lock();

            EL_LOG_FUNCTION_CALL();
            if (subscription_ptr != nullptr)
                subscription_ptr->cancel();
        }

        /**
         * @brief cancels the subscription even if there
         * are still references to the subscription_hdl.
         * Usually, this is not required.
         * 
         * @attention (external entry: public method)
         */
        void cancel()
        {
            auto lock = ctx.get_lock();

            if (subscription_ptr != nullptr)
                subscription_ptr->cancel();
        }
    };
    
    // shortcut for shared pointer to subscription handle which is the object actually returned to user code
    template<class _SUB_T>
    using subscription_hdl_ptr = std::shared_ptr<subscription_hdl<_SUB_T>>;

    /**
     * @brief shortcut for shared_ptr to subscription_hdl for event_subscription.
     * This type is library user-facing, so this is a short alias.
     */
    using event_sub_hdl_ptr = subscription_hdl_ptr<event_subscription>;
} // namespace el::msglink
