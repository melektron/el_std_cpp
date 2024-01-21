/*
ELEKTRON © 2024 - now
Written by melektron
www.elektron.work
21.01.24, 20:10
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink procedure class used to define custom procedures
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
     * @brief base class for all incoming msglink procedure
     * definition classes. To create an incoming procedure define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_INCOMING_PROCEDURE macro to generate the required boilerplate.
     * Incoming procedures must additionally have one structure called "parameters" which must be an el::decodable
     * and one structure called "results" which must be an el::encodable. They can be defined with
     * default codable definition macros.
     */
    struct incoming_procedure
    {
        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_incoming_proc_dummy() const noexcept = 0;
    };

    /**
     * @brief base class for all outgoing msglink procedure
     * definition classes. To create an outgoing procedure define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_OUTGOING_PROCEDURE macro to generate the required boilerplate.
     * Outgoing procedures must additionally have one structure called "parameters" which must be an el::encodable
     * and one structure called "results" which must be an el::decodable. They can be defined with
     * default codable definition macros.
     */
    struct outgoing_procedure : public el::encodable
    {
        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept = 0;
    };

    /**
     * @brief shortcut base class for all bidirectional (incoming and outgoing) msglink procedure 
     * definition classes. It is simply a composite class inheriting form outgoing_procedure
     * and incoming_procedure to save you the hassle of having to do that manually. 
     * Derived classes must satisfy all the requirements of incoming and outgoing procedures.
     * To create a bidirectional procedure define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_BIDIRECTIONAL_PROCEDURE macro to generate the required boilerplate.
     */
    struct bidirectional_procedure : public incoming_procedure, public outgoing_procedure
    {};


#ifdef __EL_ENABLE_CXX20
    /**
     * The following concepts define constraints that allow targeting specific
     * kinds of procedures such as an procedure class that is either ONLY an incoming
     * procedure or ONLY an outgoing procedure or a bidirectional procedure (BOTH incoming
     * and outgoing)
     */

    /**
     * @brief Constrains _ET to be ONLY derived from incoming_procedure
     * and NOT from outgoing_procedure
     */
    template<class _ET>
    concept IncomingOnlyProcedure = std::derived_from<_ET, incoming_procedure> && !std::derived_from<_ET, outgoing_procedure>;

    /**
     * @brief Constrains _ET to be at derived at least from incoming_procedure
     * (but can additionally also derive from outgoing_procedure)
     */
    template<class _ET>
    concept AtLeastIncomingProcedure = std::derived_from<_ET, incoming_procedure>;

    /**
     * @brief Constrains _ET to be ONLY derived from outgoing_procedure
     * and NOT from incoming_procedure
     */
    template<class _ET>
    concept OutgoingOnlyProcedure = std::derived_from<_ET, outgoing_procedure> && !std::derived_from<_ET, incoming_procedure>;

    /**
     * @brief Constrains _ET to be at derived at least from outgoing_procedure
     * (but can additionally also derive from incoming_procedure)
     */
    template<class _ET>
    concept AtLeastOutgoingProcedure = std::derived_from<_ET, outgoing_procedure>;

    /**
     * @brief Constrains _ET to be derived BOTH from incoming_procedure
     * and from outgoing_procedure, making it a bidirectional procedure
     */
    template<class _ET>
    concept BidirectionalProcedure = std::derived_from<_ET, incoming_procedure> && std::derived_from<_ET, outgoing_procedure>;

    /**
     * @brief Constrains _ET to be derived from incoming_procedure
     * and or outgoing_procedure. This constrains a type to be any
     * sort of procedure.
     */
    template<class _ET>
    concept AnyProcedure = std::derived_from<_ET, incoming_procedure> || std::derived_from<_ET, outgoing_procedure>;

#endif  // __EL_ENABLE_CXX20


// (public) generates the necessary boilerplate code for an incoming procedure class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_INCOMING_PROCEDURE(TypeName, ...)                         \
    static inline const char *_procedure_name = #TypeName;                          \
    virtual void _el_msglink_is_incoming_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, parameters>::value,                        \
        "incoming procedure parameters type must be decodable"                      \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::encodable, results>::value,                           \
        "incoming procedure results type must be encodable"                         \
    );


// (public) generates the necessary boilerplate code for an outgoing procedure class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_OUTGOING_PROCEDURE(TypeName, ...)                         \
    static inline const char *_procedure_name = #TypeName;                          \
    virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::encodable, parameters>::value,                        \
        "incoming procedure parameters type must be encodable"                      \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, results>::value,                           \
        "incoming procedure results type must be decodable"                         \
    );


// (public) generates the necessary boilerplate code for a procedure class.
// The first and only argument is the structure type itself which is used to get the name.
#define EL_MSGLINK_DEFINE_BIDIRECTIONAL_PROCEDURE(TypeName, ...)                    \
    static inline const char *_procedure_name = #TypeName;                          \
    virtual void _el_msglink_is_incoming_proc_dummy() const noexcept override {}    \
    virtual void _el_msglink_is_outgoing_proc_dummy() const noexcept override {}    \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, parameters>::value &&                      \
        std::is_base_of<::el::encodable, parameters>::value,                        \
        "bidirectional procedure parameters type must be en- and decodable"         \
    );                                                                              \
    static_assert(                                                                  \
        std::is_base_of<::el::decodable, results>::value &&                         \
        std::is_base_of<::el::encodable, results>::value,                           \
        "bidirectional procedure results type must be en- and decodable"            \
    );


} // namespace el::msglink