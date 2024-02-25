/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
23.02.24, 09:38
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

Context class, of which an instance is created by the communication class and that is shared with all
objects related to the msglink communication class tree instance, such as client handlers and links.
This class contains global context data required for communication
*/

#pragma once

#include <mutex>


namespace el::msglink
{
    class ct_context
    {
    public: // types

        // type of the global class tree lock
        using lock_type_t = std::unique_lock<std::mutex>;

    private:
        // mutex to guard the state of the entire msglink communication class tree and make
        // it entirely thread safe.
        // This has to be locked at the beginning of every public method call or other external entry 
        // into the class tree (such as asio callback). Lock using get_lock() method.
        std::mutex master_guard;

    public:

        ct_context() = default;

        /**
         * @brief acquires a lock on the master class tree guard
         * and returns it. The lock is held until the object is destructed.
         * 
         * @return lock_type_t (std::unique_lock<std::mutex>)
         */
        lock_type_t get_lock()
        {
            return lock_type_t(master_guard);
        }
    };
    
} // namespace el::msglink

