/*
ELEKTRON © 2025 - now
Written by melektron
www.elektron.work
21.02.25, 09:47
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Statically allocated C++ stack container class.
*/

#pragma once

#include <initializer_list>
#include <cstdint>
#include <type_traits>

#include "cxxversions.h"
#ifdef __EL_ENABLE_CXX11

namespace el
{
    template<typename _T, std::size_t _Nmax>
    class static_stack
    {
    public:
        // allow class with different template configuration to access
        // protected members for copy and move initialization of different-sized stacks
        template <typename _OT, std::size_t _ONmax>
        friend class static_stack;

        typedef _T                  value_type;
        typedef value_type*         pointer;
        typedef const value_type*   const_pointer;
        typedef value_type&         reference;
        typedef const value_type&   const_reference;
        typedef value_type*         iterator;
        typedef const value_type*   const_iterator;
        typedef std::size_t         size_type;

    protected:
        // memory buffer for holding up-to _Nmax elements
        alignas(_T) std::uint8_t element_buffer[sizeof(_T) * _Nmax];
        // how many elements are currently stored in the buffer
        size_type current_len = 0;
        // whether a recent operation would have resulted in an overflow
        bool overflow_flag = false;

        /*
        construct empty
        construct copy
        construct move
        + empty
        + size
        + back
        push_back (copy ref)
        push_back (move ref)
        emplace_back (move ref args)
        pop_back
        */
    public:

        /**
         * @brief Construct a new empty static stack object
         */
        static_stack()
        {}

        /**
         * @brief Construct a new static stack while
         * copying all elements from the input iterator that fit
         * in the stack size. If the stack is too small, the
         * overflow flag is set and not more elements are copied.
         * 
         * @tparam _Iter iterator type
         * @param _first iterator pointing to the first element to copy
         * @param _last iterator pointing to one past the last element to copy
         */
        template<typename _Iter>
        static_stack(_Iter _first, _Iter _last)
        {
            for (_Iter it = _first; it != _last; ++it)
            {
                emplace(*it);
                if (overflow_flag)
                    break;
            }
        }

        /**
         * @brief Construct a new static stack while
         * copying all objects from another stack
         * 
         * @param _other the stack to copy
         */
        template<std::size_t _ONmax>
        static_stack(const static_stack<_T, _ONmax> &_other)
            : static_stack(_other.begin(), _other.end())
        {}

        /**
         * @brief Construct a new static stack while
         * moving all objects from the other stack to this one.
         * Since the stack is statically allocated, the actual
         * data on the stack is only pseudo-moved, as it needs
         * to be copied into the the memory footprint of the new instance.
         * Each stack element is however moved itself, so if any stack element
         * benefits from move semantics those will be used.
         * @note This will move as many elements as possible from 
         * the source stack (_other) to the new instance. After the move,
         * the instances in the old stack are destroyed and the stack instance
         * is left in a state of size() == 0. If there is not enough space
         * in the new stack to accommodate all elements form the other stack,
         * the overflow flag will be set and the elements will still be 
         * "moved" out of the old instance and destroyed, just not be added to 
         * the new instance. These elements are lost to the ether and both
         * stacks are left in valid state with _other.size() == 0 as describe 
         * above.
         * 
         * @param _other stack to move from.
         */
        template<std::size_t _ONmax>
        static_stack(static_stack<_T, _ONmax> &&_other)
        {
            for (auto it = _other.begin(); it != _other.end(); ++it)
            {
                // trie to move until we have overflowed, by then we 
                // don't needto try for each element to follow.
                if (!overflow_flag)
                    emplace(std::move(*it));
                
                // manually destroy the element in the source stack 
                // after move, even if they don't fit in the new stack.
                it->~_T();
            }

            // there are now no longer any elements in the source stack
            _other.current_len = 0;
        }


        /**
         * @brief Construct a new static stack by copying
         * elements from an initializer list. The first
         * element in the list will be at the bottom of the stack,
         * the last element will end up at the top.
         * If there is not enough space for all elements, the overflow
         * flag is set and the copying of further elements is aborted.
         */
        static_stack(std::initializer_list<value_type> _il)
        {
            for (const auto &el : _il)
            {
                emplace(el);
                if (overflow_flag)
                    break;
            }
        }

        /**
         * @brief Destroys all elements
         */
        ~static_stack()
        {
            // destroy all elements on the stack
            while (pop());
        }

        /**
         * @return std::size_t the amount of elements currently in the container
         */
        inline size_type size() const noexcept
        {
            return current_len;
        }

        /**
         * @return true if the there are zero elements in the container
         */
        inline bool empty() const noexcept
        {
            return current_len == 0;
        }

        /**
         * @return true if there is no more space for more elements on the stack.
         * Adding a new element in that case would cause an overflow exception
         */
        inline bool full() const noexcept
        {
            return current_len >= _Nmax;
        }

        /**
         * @retval true stack has overflowed in a previous operation and was not cleared since then
         * @retval false stack has not been overflowed
         */
        inline bool overflowed() const noexcept
        {
            return overflow_flag;
        }

        /**
         * @brief resets the overflow flag. To be called after handling 
         * an overflow condition.
         */
        inline void clear_overflow() noexcept
        {
            overflow_flag = false;
        }

        /**
         * @return mutable iterator to the beginning of the stack.
         * If the stack is element this does not point to a valid element.
         */
        iterator begin() noexcept
        {
            return (iterator)element_buffer;
        }

        /**
         * @return const_iterator to the beginning of the stack.
         * If the stack is element this does not point to a valid element.
         */
        const_iterator begin() const noexcept
        {
            return (const_iterator)element_buffer;
        }

        /**
         * @return mutable iterator pointing the the element after the
         * top of the stack. This should never be dereferenced.
         * If the stack is element this is the same as begin().
         */
        iterator end() noexcept
        {
            return ((iterator)element_buffer) + current_len;
        }

        /**
         * @return const_iterator pointing the the element after the
         * top of the stack. This should never be dereferenced.
         * If the stack is element this is the same as begin().
         */
        const_iterator end() const noexcept
        {
            return ((const_iterator)element_buffer) + current_len;
        }

        /**
         * @return mutable iterator to the last element in the container.
         * Note: This directly points to the element on the top of the stack,
         * not one past the top, such as end. If the stack is empty however, 
         * end() will be returned.
         */
        iterator top() noexcept
        {
            if (empty())
                return end();
            else
                return ((iterator)element_buffer) + (current_len - 1);
        }

        /**
         * @return const_iterator to the last element in the container.
         * Note: This directly points to the element on the top of the stack,
         * not one past the top, such as end. If the stack is empty however, 
         * end() will be returned.
         */
        const_iterator top() const noexcept
        {
            if (empty())
                return end();
            else
                return ((const_iterator)element_buffer) + (current_len - 1);
        }

        /**
         * @brief pushes a new element while constructing it in-place
         * 
         * @param __args arguments forwarded to constructor
         * @return Iterator pointing to the created element (new top()).
         * If there was no space, the overflow flag is set and end() is returned.
         * No memory will actually be corrupted.
         */
        template<typename... _Args>
        iterator emplace(_Args&&... __args)
    	{
            if (full())
            {
                overflow_flag = true;
                return end();
            }
            return new (((iterator)element_buffer) + current_len++) _T(std::forward<_Args>(__args)...);
        }

        /**
         * @brief Adds a copy of the provided data to the 
         * top of the stack
         * 
         * @param _x data to be added
         * @return Iterator pointing to the created element (new top()).
         * If there was no space, the overflow flag is set and end() is returned.
         * No memory will actually be corrupted.
         */
        iterator push(const value_type& _x)
        {
            return emplace(_x);
        }

        /**
         * @brief Attempts to moves data onto the stack.
         * 
         * @param _x data to be added
         * @return Iterator pointing to the created element (new top()).
         * If there was no space, the overflow flag is set, end() is returned
         * and the source instance is still valid (is not moved).
         * No memory will actually be corrupted.
         */
        iterator push(value_type&& _x)
        {
            return emplace(std::move(_x));
        }

        /**
         * @brief Attempts to remove and destroy the element on
         * the top of the stack. 
         * 
         * @retval true Element was removed and destroyed.
         * @retval false Stack was empty, nothing happened.
         */
        bool pop()
        {
            if (empty()) return false;
            (((iterator)element_buffer) + (--current_len))->~_T();
            return true;
        }

        template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
        friend bool operator==(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs);

        template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
        friend inline bool operator<(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs);

    };
} // namespace el

/**
 * @retval true both stacks are of the same length and all elements are equal.
 * @retval false either the length or some of the elements are not equal.
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
bool operator==(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{
    if (_lhs.size() != _rhs.size())
        return false;
    auto itl = _lhs.begin();
    auto itr = _rhs.begin();
    while (itl != _lhs.end())
    {
        if (*itl != *itr)
            return false;
        itl++;
        itr++;
    }
    return true;
}

/**
 * @retval true either the length or some of the elements are not equal.
 * @retval false both stacks are of the same length and all elements are equal.
 * @note based on operator==
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
bool operator!=(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{ 
    return !(_lhs == _rhs); 
}

/**
 * @return _lhs.size() < _rhs.size()
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
inline bool operator<(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{
    return _lhs.size() < _rhs.size();
}

/**
 * @brief _lhs.size() > _rhs.size();
 * @note based on operator<
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
inline bool operator>(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{
    return _rhs < _lhs;
}

/**
 * @brief _lhs.size() <= _rhs.size();
 * @note based on operator<
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
inline bool operator<=(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{
    return !(_rhs < _lhs);
}

/**
 * @brief _lhs.size() >= _rhs.size();
 * @note based on operator<
 */
template<typename _Tp1, std::size_t _Nmax1, std::size_t _Nmax2>
inline bool operator>=(const el::static_stack<_Tp1, _Nmax1>& _lhs, const el::static_stack<_Tp1, _Nmax2>& _rhs)
{
    return !(_lhs < _rhs);
}

#endif