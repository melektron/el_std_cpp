/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
11.11.23, 23:00
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree.

msglink event class used to define custom events
*/

#pragma once

#include "../codable.hpp"


namespace el::msglink
{


    /**
     * @brief base class for all incoming msglink event
     * definition classes. To create an incoming event define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_INCOMING_EVENT macro to generate the required boilerplate.
     * Incoming events are el::decodable[s],
     * meaning they must be decodable from json. If a decoder for a member cannot
     * be generated automatically or needs to be altered, the EL_DECODER macro can be
     * used like with codables to manually define the decoder.
     */
    struct incoming_event : public el::decodable
    {
        virtual ~incoming_event() = default;
        
        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_incoming_dummy() const noexcept = 0;
    };

// (public) generates the necessary boilerplate code for an incoming event class.
// The members listed in the arguments will be made decodable using el::decodable
// and are part of the event's data.
#define EL_MSGLINK_DEFINE_INCOMING_EVENT(TypeName, ...)                             \
    static inline const char *_event_name = #TypeName;                              \
    virtual void _el_msglink_is_incoming_dummy() const noexcept override {}         \
    EL_DEFINE_DECODABLE(TypeName, __VA_ARGS__)

    /**
     * @brief base class for all outgoing msglink event 
     * definition classes. To create an outgoing event define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_OUTGOING_EVENT macro to generate the required boilerplate.
     * Outgoing events are el::encodable[s],
     * meaning they must be encodable to json. If a encoder for a member cannot 
     * be generated automatically or needs to be altered, the EL_ENCODER macro can be
     * used like with codables to manually define the encoder.
     */
    struct outgoing_event : public el::encodable
    {
        virtual ~outgoing_event() = default;

        // dummy method which must be overriden to ensure the correct
        // generate macro is used.
        virtual void _el_msglink_is_outgoing_dummy() const noexcept = 0;
    };

// (public) generates the necessary boilerplate code for an outgoing event class.
// The members listed in the arguments will be made encodable using el::encodable
// and are part of the event's data.
#define EL_MSGLINK_DEFINE_OUTGOING_EVENT(TypeName, ...)                             \
    static inline const char *_event_name = #TypeName;                              \
    virtual void _el_msglink_is_outgoing_dummy() const noexcept override {}         \
    EL_DEFINE_ENCODABLE(TypeName, __VA_ARGS__)

    /**
     * @brief base class for all bidirectional (incoming and outgoing) msglink event 
     * definition classes. It is simply a composite class of outgoing_event
     * and incoming_event. This class must satisfy all the requirements of incoming and outgoing
     * events.
     * To create a bidirectional event define a class inheriting from this one.
     * Then use the EL_MSGLINK_DEFINE_EVENT macro to generate the required boilerplate.
     */
    struct event : public incoming_event, public outgoing_event
    {
        virtual ~event() = default;
    };

// (public) generates the necessary boilerplate code for an event class.
// The members listed in the arguments will be made codable using el::codable
// and are part of the event's data.
#define EL_MSGLINK_DEFINE_EVENT(TypeName, ...)                                      \
    static inline const char *_event_name = #TypeName;                              \
    virtual void _el_msglink_is_incoming_dummy() const noexcept override {}         \
    virtual void _el_msglink_is_outgoing_dummy() const noexcept override {}         \
    EL_DEFINE_CODABLE(TypeName, __VA_ARGS__)

} // namespace el::msglink
