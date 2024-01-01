# Message Link

Message link (msglink) is a custom networking protocol based on websockets that can be used to easily exchange data, events and commands between multiple systems over the network.  


## Principles

msglink tries to conform to the following core principles:

- Simple API: It should be easy to start a msglink server and define client handling functionality without needing to decide between a variety of different transport protocols and other config
- Clean and user-friendly API definition: event definition and use should be as simple as possible leveraging language specific features to abstract away the implementation details. There should be as little repeated boilerplates as possible.
- Platform independence: the protocol should be usable on any OS or system using multiple programming languages (protocol features should not depend on special language functions)
- Simple state management: the user program shall not be required to manage any state related to the communication such as channel subscriptions or alike. A user should be able to say "I want this specific data" to the library and the program must make sure these requests are handled even after reconnects or similar networking events.


## Features

msglink is similar to Socket.IO, except it provides additional functionality extending the simple event system as follows: 

- Strict types and data validation
- Type-defined events
- Data subscriptions
- Remote Procedure calls (with return data)


## Strict types and data validation

One of the most annoying (and often repetitive) coding tasks when it comes to network communication is serialization and deserialization (especially when communicating between different languages and platforms). At this point, JSON has become the defacto standard for most general purpose web APIs and many high-level communication protocols (at least where ever maximum performance and throughput is not the most important point). While JSON is a nice way to encode data, readable with relative ease by humans and computers likewise, it still remains a ton of work to encode and decode data in your application from or to the language-internal format. 

Most modern programming languages provide some sort of native or third party support for serializing and deserializing JSON, but the problem with JSON data received via the network is, that it is fully dynamic. At the time of development you cannot be sure what the JSON object finally will contain. So after parsing, it is required to manually go through the JSON object, checking that all it's fields match the type and restrictions required by your program. Then the data should ideally be extracted into some form of language-specific format like a struct in C/C++. 

Luckily though, there are libraries that can help us simplify this task. The Python library PyDantic provides a way to elegantly define a JSON property's type, value restrictions and optional (de)serializer functions and enables simple parsing and automatic validation of incoming data. Since everything is represented by classes, static type checkers can see the datatype of properties and provide excellent editor support. In Swift, the Codable protocol is natively supported and provides similar functionality. In some languages like C++ this is not quite as simple to represent but we can still simplify the process. 

msglink tries to implement and require these type definitions natively in it's implementation libraries, each in the style and with the features supported by the respective programming language. This way, every event has a clearly defined data structure passed along the event. Event listeners can access incoming data in the language-native format and rely on the fact that they receive what they expect. Event emitters on the other hand can pass data in the language-native format and will be forced to only emit valid data for a specific event.


## Type-defined events

Traditionally, events have been identified by a simple string, it's name. There is nothing inherently wrong with this approach, but it introduces additional places to make mistakes. One may want to listen to the same event in multiple places of a program but might make a typo when identifying the event name or forget to update a listener after changing the name. 

Language features such as enums, constants or TS literal types will solve this issue. msglink aims to integrate this as a requirement in it's implementation. This goes hand-in-hand nicely with the previous point, strict types. Every event has to have a defined and validatable data type which also defines the name of the event it is associated with. After defining it once, this event type can be used everywhere in the program (details depend on language implementation) to refer to the specific event. Typos in the event name can be avoided and it is impossible to emit events with the wrong data structure.


## Data subscriptions

Everything up to this point has just been library implementation specific improvements to the API of Socket.IO, but msglink also provides some additional features that are completely new to solve common problems in a repeatable way.

Data subscriptions basically allow a communication party to "subscribe" to a certain data source in THE OTHER PARTY's ENVIRONMENT. At first, this may sound similar to event listeners, however there is a key difference. In the usual event system, party A doesn't know what events party B listens to. Whenever a communication party has something to offer, it simply emits an event and the other party decides if it is interested in it.

Data subscriptions on the other hand, allow one party (we will call it the client, although it doesn't matter which one is the client and which one is the server) to tell the other (the server) that it needs some data value (for example a list of all online users). The server will send the requested information to the client and automatically send an update whenever the data on the server changes in some way. This is especially useful if the server itself needs to get these value updates from some other system and needs to subscribe/listen to them on demand (or if there are too many events to just send all of them all the time).

These data sources do not need to be part of a static API definition that is hard-coded and known by both parties beforehand. For example, a client might want to know the activity status of user "xyz" and get automatic updates on it. But maybe user "xyz" doesn't even exist. The client can still request the data source from the server, and if the server can provide it, it will do so. If the server cannot provide the data source, it will respond with a not-available error or send the data as soon as it is available, depending on what is required. 

The same functionality can be implemented with simple messages, however since this is used so frequently, it gets repetitive and error-prone quite quickly. By implementing this feature in a library, the repetitive parts can be abstracted and we can even use language/framework specific features to make such "remote" data sources even more convenient to use (e.g. React State or Svelt Stores).


## Remote Procedure calls

Another common use-case for messaging protocols is a remote procedure call. A remote procedure call consists of one communication party sending some data to the other one and causing some code to run there. Once the other party is finished, it will return the result to the calling party. 

A remote procedure call can be implemented by emitting an event and running some action in the even listener. At the end of the listener, another event has to be emitted containing the result returned to the client. 

There are a few problems with manually implementing this using events. First of all, in order for the request and response events to be associated with each other, some sort of unique ID must be added by the caller that is then also returned in the response so the caller can associate the result with any particular call. Second, an application is likely to have many different procedures to be called, so the additional overhead of defining and emitting separate request and response events (with this ID management) for every procedure is quite tedious and error prone, as it involves re-implementing the same functionality multiple times.

msglink avoids this by implementing the base functionality once and providing a language-specific and clean way to define procedures in one place with input data, result data and name. This is similar to [JSON-RPC](https://en.wikipedia.org/wiki/JSON-RPC) but provides the additional data validation and automatic parsing functionality described above.


## Decision criteria

It depends on a few criteria, which of the three options provided by msglink (events, data subscriptions, RPCs) shoud be used:

- **value or incident** focus: What's more important? The actual data value or the fact that it changed?
  > If the focus is on the value of some datum, a data subscription should be used. It has different syntax to events allowing it to be easily used as a (possibly observable) remote-synced variable.
  >
  > If the focus is an incident which should cause some action to be performed by the other party, then an event is the way to go. Events also carry data, but are always associated with handler functions, so called listeners, which are called when an event is received.
  >
  > In short: The focus of events is to run some code when something happens while the focus of a data subscription is to have some data value that can be accessed at any time without worrying about updating it.
- **conditionality**: When and for how long is some data needed and when do events need to be transmitted?
  > Both event listeners and data subscriptions offer a way to "enable" and "disable" them during the lifetime of the program. For Library implementations it is strongly encouraged to use language features such as scope and object lifetime to determine when events and data subscription updates are needed in a granular way. This can save on network bandwidth.
  >
  > When listening to an event, a handler function (listener) is registered which is called whenever an event is received from the other party. Registering a listener may yield an object or handle representing it. This object can then be used to manage the lifetime of the listener. For example in C++, if an event is only needed inside a class instance, the listener object should be a member of that class and be unregistered whenever the class instance is destroyed (i.e. goes out of scope).
  >
  > When subscribing to some data using data subscriptions, a similar object/handle will be created. Again in C++, it might be used to directly access the data using the arrow operator and manage the lifetime of the data subscription like the event listener object.
- **uniqueness**: Is a piece of data/event unique or are there multiple different ones with the same structure and meaning?
  > Uniqueness means, that exactly one (entity) of something exists.
  >
  > In the msglink protocol, events (not event instances) are unique entities. Let's say, there is an event called "device_connect". In the entire application, there is only one such event with a clearly defined data structure associated with it. However, when emitting the event, the data value may be different every time. For example this event might have a "device_id" parameter which uniquely identifies the device that has joined. No matter what the device ID is, every listener for "device_connect" will receive the event.
  >
  > Sometimes you may only want a listener to be called when the device with a specific ID is connected. You cannot define a different event for each device ID, because it would be very tedious and you probably don't even known which device IDs exist at all at compile time. Instead this would require some sort of filter, comparing the actual data. This can be done inside the listener function, but has as well a big disadvantage: Even though only events with a specific device ID are required, all "device_connect" events are still transmitted over the network. And then you probably need to do the same thing with the "device_disconnect" event.
  >
  > In such a situation, what you really want is a data subscription which can have subscription parameters. So you might define a data source called "device_connected" which may have a boolean property "is_connected" which can be true or false. Then you define the subscription parameter to have a property "device_id". When this data source is subscribed, a device ID has to be passed to that call. The providing party can then immediately respond saying that it either can or cannot provide the data for the given device ID. If it can, it will then update the online value for only that device ID and all others will not be transmitted over the network.
- **confirmation**: Does some event require any form of confirmation/response/result from the other party?
  > When the goal is for one communication party to cause some sort of action by the other party, an event can be used. 
  >
  > However, sometimes the executing party needs to send some result data or outcome of the action back to the emitter. In the past, it was necessary to define a separate request and response event and write code for every type of interaction to sync the two up, wait for the response and so on. This is very tedious and repetitive.
  >
  > With msglink, for such a case a procedure can be defined instead of an event. A procedure is basically two events combined, with the only difference being that the listener now returns another object which is sent back to the emitter. This can be integrated nicely with the async programming capability of many programming languages.
  >
  > Another difference between procedures and events is the way that they are handled on the receiving side. Since events have no way of returning data or results to the emitter, there may be many listeners that are notified of the event and can perform actions when that happens. Although each of them may cause various actions, like emitting more events as a response, none of them are responsible for or capable of defining one singular "outcome" or "result" of the event. This is a broadcast theme.
  >
  > With procedures, this is different. Since a procedure has to have one single definite result to be sent back to the caller (roughly equivalent to emitter for events) after it has been handled, there can be only one handler. A procedure may be called from many different places, but on the receiving side, there has to be exactly one handler (function). Since it is necessary for a complete transaction to always return a result, it is also not allowed for there to be no handler at all. So procedures always have exactly one handler.


# Protocol details

In the following section, the details of the communication protocol will be described. Every communication step will be accompanied with an example of how the corresponding websocket message will look like. 

As an example we are using a system managing multiple hardware devices connected to some computer. A webapp (client) displays information about connected devices and allows managing them by communicating with a server running on that computer.

The client needs the following:

- to be informed when an error occurs<br>
  **Indicent without response -> event: "error_occurred"**
- a list of devices connected to the computer identified by their ID<br>
  **unique data -> data subscription (simple): "devices"**
- the power consumption of the device currently displayed in the UI<br>
  **non-unique data depending on parameter -> data subscription (with parameter): "power_consumption"**
- the ability for the user to disable a device, for example because it needs to much power<br>
  **Command with response -> RPC: "disable_device"**

> In msglink there is (almost) no difference between the client and the server except for (a) how the socket connection is established and (b) transaction IDs (which are covered below). Therefore, any example described here could just as well work in the other direction.


## The basics

As explained in the beginning, msglink uses websockets as the underlying communication layer. The websocket protocol already has the concept of messages, which is very convenient. Websockets guarantee that messages transmitted by one communication party are received as a whole and in order (no byte-stream fiddling needed). These messages are the underlying protocol data units (PDUs) used by msglink protocol.

For now, msglink uses json to encode protocol data and user data in websocket messages. Later versions may introduce binary encoding if that turns out to be necessary for performance reasons.

### Working message

Working messages are just the "normal" messages sent back and forth while the connection is open.

Every message has 2 base properties:

```json
{
    "type": "...",  // string
    "tid": 123,     // int
    ...
}
```

The **```type```** property defines the purpose of the message. There are the following message types:

- auth
- auth_ack
- evt_sub
- evt_unsub
- evt_emit
- data_sub
- data_sub_ack
- data_sub_nak
- data_unsub
- data_change
- rpc_call
- rpc_nak
- rpc_err
- rpc_result

The **```tid```** property is the transaction ID. The transaction ID is a signed integer number which (within a single session) uniquely identifies the transaction the message belongs to. 

> A transaction is a (from the perspective of the protocol implementation) complete interaction between the two communication parties. It could be an event, a data subscription or an RPC. <br>
This is a scheme used by many networking protocols and is required for the communication parties to know what messages belong together when a single transaction requires multiple back-and-forth messages like during an RPC. This is one of those tedious repetitive things that otherwise would need to be reimplemented for every command-response event pair if it was implemented manually using only events. 

Every time a communication party starts a new interaction, it first generates a new transaction ID by using an internal ID counter. To prevent both parties from generating the same transaction ID at the same time, the **server always starts at transaction ID 1 and increments it** for each new transaction it starts (1, 2, 3, 4, ...) while the **client always starts a transaction ID -1 and decrements it** from there (-1, -2, -3, -4, ...). Eventually, the two will meet "in the middle" when the integer overflows, which will take a very long time (assuming 64 bit or even 32 bit integers).

The names of properties are - intentionally - kept as short as possible while still being readable pretty well by humans to reduce message size.

Messages can have other properties specific to the message type.


### Closing message

When closing the msglink and therefore websocket connection, custom close codes and reasons are used. The following table describes the possible codes and their meaning:

| Code | Meaning | Notes |
|---|---|---|
| 1000 | Closed by user | Reason string is user defined.
| 3001 | msglink version incompatible | |
| 3002 | link version mismatch | |
| 3003 | Event requirement(s) unsatisfied | |
| 3004 | Data source requirement(s) unsatisfied | |
| 3005 | RPC requirement(s) unsatisfied | |


## Authentication procedure 

When a msglink client first connects to the msglink server both parties send an initial JSON encoded authentication message to the other party containing the following information:

```json
{
    "type": "auth",
    "tid": 1,  // should be 1 for server and -1 for client according to definition of tid generation above
    "proto_version": [1, 2, 3],
    "link_version": 1,
    "events": ["error_occurred"],
    "data_sources": ["devices", "power_consumption"],
    "procedures": ["disable_device"]
}
```

- **```proto_verison```**: the msglink protocol version (determines whether certain features are supported)
- **```link_verison```**: the user-defined link version (version of the user defined protocol)
- **```events```**: a list of events the party may emit (it's outgoing events)
- **```data_sources```**: a list of data sources the party can provide (it's outgoing data sources)
- **```procedures```**: a list of remote procedures the party provides

After receiving the message from the other party, both parties will check that the protocol versions of the other party are compatible and that the user defined link versions match. If that is not the case, the connection will be closed with code 3001 or 3002.

> Protocol version compatibility is determined by the party with the higher (= newer) version as that one is assumed to know of and be able to judge compatibility with the lower version. If a party receives an auth message with a higher protocol version than it's own, it skips the version compatibility check.

The message also contains lists of all the functionality the party can provide to the other one. These lists are used by the receiving party to determine weather they fulfill all it's requirements. If any requirement fails, the connection is immediately closed with the corresponding code described below. This helps to detect simple coding mistakes early and reduce the amount of errors that will occur later during communication.

- **events**: one party's incoming event list must be a subset of the other's outgoing event list. Fails with code 3003. Fail reasons:
  - If one party may want to listen for an event the other party doesn't even know about and will never be able to emit
- **data sources**: one party's data subscription list must be a subset of the other's data source list. Fails with code 3004. Fail reasons:
  - If one party may subscribe to a source the other doesn't know about and provide
- **remote procedure calls**: one party's called procedures list must be a subset of the other's callable procedure list. Fails with code 3005. Fail reasons:
  - If one party may call a procedure the other doesn't know about and cannot handle

Obviously these requirements are only checked approximately. The client doesn't know at that point whether the server ever will emit the "error_occurred" event or even if there will ever be a listener for it. The only thing it knows is that both the server and itself know that this event exists and know how to deal with it should that become necessary later. 

If no problems were found, each party sends an authentication acknowledgement message as a response to the other with the respective transaction ID (not a new one) to complete the authentication transaction:

```json
{
    "type": "auth_ack",
    "tid": 1    // now 1 for client, -1 for server
}
```

Only after both parties' authentication transactions have been successfully completed, is either party allowed to send further messages. This is defined by one party as both:

- having sent the auth_ack message in response to the other's auth message
- having received the auth_ack message in response to it's own auth message


## Event messages

If a communication party has a listener for a specific event, it needs to first subscribe to the event before it will receive it over the network. To do so, the event subscribe message is sent:

```json
{
    "type": "evt_sub",
    "tid": ..., // new transaction ID 
    "name": "..."
}
```

- **```name```**: name of the event to be subscribed to

This assumes the event name is valid and supported by the other party, as that was already negotiated during authentication. Therefore this message is defined to **guarantee** the event will be subscribed after it is received and no response is required. 

In case this message is in fact received for an invalid event, this is due to a library implementation issue. The implementation should log this locally as a warning.

After receiving this message, the emitting party will inform the listening one when this event type is emitted using the event emit message:


```json
{
    "type": "evt_emit",
    "tid": ..., // new transaction ID for each emit
    "name": "...",
    "data": {...}
}
```

- **```name```**: name of the emitted event
- **```data```**: a json object containing the data associated with the event. This data will be validated according to the schema defined on the listening party and will cause a local error if it is invalid (error will not be sent to emitting party). Listeners are only called if the data was validated successfully.

Should one party receive an event message that it hasn't subscribed to, a local warning should be logged. This doesn't cause any harm but wastes bandwidth and is likely due to a library implementation issue which is to be fixed.

Once all listeners are disabled on the listening party, it can tell the emitting party that the event information is no longer required with the event unsubscribe message:

```json
{
    "type": "evt_unsub",
    "tid": ..., // new transaction ID 
    "name": "..."
}
```

- **```name```**: name of the event to unsubscribe from

Similar to subscribe, there are no acknowledgement messages for unsubscribe. Unsubscribe will guarantee that no more events with the given name are received. If the unsubscribed event wasn't subscribed before or doesn't even exist, a local warning is logged on the receiving party only.


# Notes 

## Naming alternatives

Note for future me: If msglink doesn't fit for some reason in the future, here are some alternative name ideas:

- msgio (MessageIO)


## Link collection

- wspp client reconnect: https://github.com/zaphoyd/websocketpp/issues/754#issue-353706390


## TODOS

```cpp
    // DONE: next up is moving link and event to el .hpp files. 
    // TODO: Then add some more macros, a separate event class and more event define functions so a user can decide wether they want events to be just en/de codable and if they should
    // be just outgoing/incoming or both.
    // Also maybe add the possibility for an event handler as a method of the event class (maybe, not sure if so many options
    // are a good idea).
    // Then actually add this link support to the msglink server.
    // Then implement a corresponding msglink client (reconnects, ...)
    // Then implement state-management calls on a link (e.g. on_connect/on_disconnect, possibly a "currently not connected but attempting to reconnect, don't give up jet" state)
    // Then add support for data subscriptions (they need more state management (e.g. requests ) in their own classes)
    // Then add support for remote procedure calls.
```