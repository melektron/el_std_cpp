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
#include <asio/io_service.hpp>

namespace el::msglink
{

    class tracking_mutex :
        public std::mutex
    {
    private:
        std::atomic<std::thread::id> m_holder = std::thread::id{};

    public:
        void lock()
        {
            std::mutex::lock();
            m_holder = std::this_thread::get_id(); 
            EL_LOGD("ct locked");
        }

        void unlock()
        {
            m_holder = std::thread::id();
            std::mutex::unlock();
            EL_LOGD("ct unlocked");
        }

        bool try_lock()
        {
            if (std::mutex::try_lock()) {
                m_holder = std::thread::id();
                return true;
            }
            return false;
        }

        /**
         * @return true if the mutex is locked by the caller of this method.
         */
        bool locked_by_caller() const
        {
            return m_holder == std::this_thread::get_id();
        }

    };

    class ct_context
    {
    public: // types
        using mutex_type_t = tracking_mutex;
        using lock_type_t = std::unique_lock<mutex_type_t>;

    private:
        // mutex to guard the state of the entire msglink communication class tree and make
        // it entirely thread safe.
        // This has to be locked at the beginning of every public method call or other external entry 
        // into the class tree (such as asio callback). Lock using get_lock() method.
        mutex_type_t master_guard;
    
    public:
        // the main io service used for communication and callback scheduling
        std::unique_ptr<asio::io_service> io_service;

    public:

        ct_context()
            : io_service(new asio::io_service())
        {
        }

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

        /**
         * @brief acquires a lock on the master class tree guard
         * and returns it unless a lock is already held by the calling thread.
         * If the lock is already held, an empty unique_lock is returned.
         * Because of this, the returned object should not be accessed with the assumption of an owning lock.
         * If the lock is held by it, it is held until the object is destroyed.
         * In any case, this function guarantees the calling thread is holding the lock
         * after return.
         * 
         * This function is used in places when it cannot be guaranteed that a call is external, such as in destructors.
         * It should only be used sparely.
         * 
         * @return lock_type_t lock if the lock was not locked jet
         */
        lock_type_t get_soft_lock()
        {
            // This operation is not atomic. It could happen, that between this owner check
            // and the following attemted lock, another thread locks the mutex. In that case, the calling
            // thread has to wait.
            // This is not a problem however. The only thing this protects against is that the same thread
            // doesn't try to re-acquire the lock.
            if (master_guard.locked_by_caller())
                return lock_type_t();
            // actually get the lock
            return lock_type_t(master_guard);
        }
    };
    
} // namespace el::msglink

