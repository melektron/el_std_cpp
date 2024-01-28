/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
21.01.24, 20:10
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink function class used to define custom remote functions
*/

#pragma once

#include "../cxxversions.h"

#include <type_traits>
#ifdef __EL_ENABLE_CXX20
#include <concepts>
#endif

#include "../codable.hpp"


namespace el::msglink
{


    /**
     * @brief base class for all incoming msglink function
     * definition classes. To create an incoming function define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_INCOMING_FUNCTION macro to generate the required boilerplate.
     * Incoming functions must additionally have one structure called "parameters_t" which must be an el::decodable
     * and one structure called "results_t" which must be an el::encodable. They can be defined with
     * default codable definition macros.
     */
    struct incoming_function
    {
        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_incoming_proc_dummy() const noexcept = 0;
    };

    /**
     * @brief base class for all outgoing msglink function
     * definition classes. To create an outgoing function define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_OUTGOING_FUNCTION macro to generate the required boilerplate.
     * Outgoing functions must additionally have one structure called "parameters_t" which must be an el::encodable
     * and one structure called "results_t" which must be an el::decodable. They can be defined with
     * default codable definition macros.
     */
    struct outgoing_function
    {
        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept = 0;
    };

    /**
     * @brief shortcut base class for all bidirectional (incoming and outgoing) msglink function 
     * definition classes. It is simply a composite class inheriting form outgoing_function
     * and incoming_function to save you the hassle of having to do that manually. 
     * Derived classes must satisfy all the requirements of incoming and outgoing functions.
     * To create a bidirectional function define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_BIDIRECTIONAL_FUNCTION macro to generate the required boilerplate.
     */
    struct bidirectional_function : public incoming_function, public outgoing_function
    {};


#ifdef __EL_ENABLE_CXX20
    /**
     * The following concepts define constraints that allow targeting specific
     * kinds of functions such as an function class that is either ONLY an incoming
     * function or ONLY an outgoing function or a bidirectional function (BOTH incoming
     * and outgoing)
     */

    /**
     * @brief Constrains _ET to be ONLY derived from incoming_function
     * and NOT from outgoing_function
     */
    template<class _ET>
    concept IncomingOnlyFunction = std::derived_from<_ET, incoming_function> && !std::derived_from<_ET, outgoing_function>;

    /**
     * @brief Constrains _ET to be at derived at least from incoming_function
     * (but can additionally also derive from outgoing_function)
     */
    template<class _ET>
    concept AtLeastIncomingFunction = std::derived_from<_ET, incoming_function>;

    /**
     * @brief Constrains _ET to be ONLY derived from outgoing_function
     * and NOT from incoming_function
     */
    template<class _ET>
    concept OutgoingOnlyFunction = std::derived_from<_ET, outgoing_function> && !std::derived_from<_ET, incoming_function>;

    /**
     * @brief Constrains _ET to be at derived at least from outgoing_function
     * (but can additionally also derive from incoming_function)
     */
    template<class _ET>
    concept AtLeastOutgoingFunction = std::derived_from<_ET, outgoing_function>;

    /**
     * @brief Constrains _ET to be derived BOTH from incoming_function
     * and from outgoing_function, making it a bidirectional function
     */
    template<class _ET>
    concept BidirectionalFunction = std::derived_from<_ET, incoming_function> && std::derived_from<_ET, outgoing_function>;

    /**
     * @brief Constrains _ET to be derived from incoming_function
     * and or outgoing_function. This constrains a type to be any
     * sort of function.
     */
    template<class _ET>
    concept AnyFunction = std::derived_from<_ET, incoming_function> || std::derived_from<_ET, outgoing_function>;

#endif  // __EL_ENABLE_CXX20


// (public) generates the necessary boilerplate code for an incoming function class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_INCOMING_FUNCTION(TypeName, ...)                          \
    static inline const char *_function_name = #TypeName;                           \
    virtual void _el_msglink_is_incoming_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, parameters_t>::value,                      \
        "incoming function parameters type must be decodable"                       \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::encodable, results_t>::value,                         \
        "incoming function results type must be encodable"                          \
    );


// (public) generates the necessary boilerplate code for an outgoing function class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_OUTGOING_FUNCTION(TypeName, ...)                          \
    static inline const char *_function_name = #TypeName;                           \
    virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::encodable, parameters_t>::value,                      \
        "incoming function parameters type must be encodable"                       \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, results_t>::value,                         \
        "incoming function results type must be decodable"                          \
    );


// (public) generates the necessary boilerplate code for a function class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_BIDIRECTIONAL_FUNCTION(TypeName, ...)                     \
    static inline const char *_function_name = #TypeName;                           \
    virtual void _el_msglink_is_incoming_proc_dummy() const noexcept override {}    \
    virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, parameters_t>::value &&                    \
        std::is_base_of<::el::encodable, parameters_t>::value,                      \
        "bidirectional function parameters type must be en- and decodable"          \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, results_t>::value &&                       \
        std::is_base_of<::el::encodable, results_t>::value,                         \
        "bidirectional function results type must be en- and decodable"             \
    );


} // namespace el::msglink