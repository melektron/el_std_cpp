# Message Link

Message link (msglink) is a custom networking protocol based on websockets that can be used to easily exchange data, events and commands between multiple systems over the network. 


## Principles

msglink tries to conform to these core principals:

- Simple API: It should be easy to start a msglink server and define client handling functionality without needing to decide between a thousand different transport protocols and other config
- Clean and user-friendly API definition: event definition and use should be as simply as possible leveraging language specific features to abstract away the implementation details. There should be as little repeated boiler-plate as possible.
- Platform independence: the protocol should be usable on any OS or system using multiple programming languages (protocol features should not depend on special language functions)
- Simple state management: The user program shall not be required to manage any state related to the communication such as channel subscriptions or any of that stuff. A user should be able to say "I want this and this and this data" to the library and it must make sure these requests are fulfilled even after reconnects and similar incidents.


## Features

msglink is similar to Socket.IO, except it provides additional functionality extending the simple event system. 

- Strict types and data validation
- Type-Defined events
- Data subscriptions
- Remote Procedure calls (with return data)


## Strict types and data validation

One of the most annoying and often repetitive coding tasks when it comes to network communication is serialization and deserialization (especially when communicating between different languages and platforms). At this point, JSON has become the defacto standard for most general purpose web APIs and many high-level communication protocols (at least where ever maximum performance and throughput is not the most important point). While JSON is a nice way to encode data both readable with relative ease by humans and computers, it is still a ton of work to encode and decode data in your application from/to the language-internal format. 

Most modern programming languages provide some sort of native or third party support for serializing and deserializing JSON, but the problem with JSON data received via the network is, that it is fully dynamic. You cannot be sure at the time of development what the JSON object will contain. So after parsing, it is required to manually go through the JSON object, checking that all it's fields match the type and restrictions required by your program. Then the data should ideally be extracted into some form of language-specific format like a struct in C/C++. 

Luckily there are libraries that can help us simplify this task. The Python library PyDantic provides a way to elegantly define a JSON property's the type, value restrictions and optional (de)serializer functions and enables simple parsing and automatic validation of incoming data. Since everything is represented by classes, static type checkers can see the datatype of properties and provide excellent editor support. In Swift, the Codable protocol is natively supported and provides similar functionality. In some languages like C++ this is not quite as simple to represent but we can still simplify the process. 

msgpack tries to implement and require these type definitions natively in it's implementation libraries, each in the style and with the features supported by the respective programming language. This way, every event has a clearly defined data structure passed along the event. Event listeners can access incoming data in the language-native format and rely on the fact that they receive what they expect. Event emitters on the other hand can pass data in the language-native format and will be forced to only emit valid data for a specific event.


## Type-defined events

Traditionally, events have been identified by a simple string, it's name. There is nothing inherently wrong with this approach, but it introduces additional places to make mistakes. One may want to listen to the same event in multiple places of a program but might make a typo when identifying the event name or forget to update one listener after changing the name. 

Language features such as enums, constants or TS literal types will solve this issue. However, msglink aims to integrate this as a requirement in it's implementation. This goes hand-in-hand nicely with the previous point, strict types. Every event has to have a defined and validatable data type which also defines the name of the event it is associated with. After defining it once, this event type can be used everywhere in the program (details depend on language implementation) to refer to this specific event, there cannot be typos in the event name and it is impossible emit events with the wrong data structure.


## Data subscriptions

Everything up to this point has just been library implementation specific improvements to the API of Socket.IO, but msglink also provides some additional features that are completely new to solve common problems in a repeatable way.

Data subscriptions basically allow a communication party to "subscribe" to a certain data source in THE OTHER PARTY's ENVIRONMENT. At first, this may sound similar to event listeners, however there is a key difference. In the usual event system, party A doesn't know what events party B listens to. Whenever a communication party has something to offer, it simply emits an event and the other party decides if it is interested in it.

Data subscriptions on the other hand, allow one party (we will call it the client although it doesn't matter which one is the client and which one is the server) to tell the other (the server) that it needs some data value (for example a list of all online users). The server will then send the client the requested information and also automatically send an update whenever the data changes in some way on the server. This is especially useful if the server itself needs to get these value updates from some other system and needs to subscribe/listen to them on demand (or if there are too many events to just send all of them all the time).

These data sources do not need to be part of a static API definition that is hard-coded and known by both parties beforehand. For example, a client might want to know the activity status of user "xyz" and get automatic updates on it. But maybe user "xyz" doesn't even exist. The client can still request the wanted data source from the server, and if the server can provide it it will do so. If the server cannot provide the data source, it will respond with a not-available error or send the data as soon as it is available, depending on what is required. 

The same functionality can be implemented with simple messages, however since this is used so frequently, it gets repetitive and error-prone quite quickly. By implementing this feature in a library, the repetitive parts can be abstracted and we can even use language/framework specific features to make such "remote" data sources even more convenient to use (e.g. React State or Svelt Stores).


## Remote Procedure calls

Another common use-case for messaging protocols is a remote procedure calls. A remote procedure call consists of one communication party sending some data to the other one and causing some code to run there. Once the other party is finished, it will return the result to the calling party. 

A remote procedure call can be implemented by emitting an event and running some action in the even listener. At the end of the listener, another event has to be emitted containing the result returned to the client. 

There are a few problems with manually implementing this using events. First of all, in order for the request and response events to be associated with each other, some sort of unique ID must be added by the caller that is then also returned in the response so the caller can associate the result with any particular call. Second, an application is likely to have many different procedures to be called, so the additional overhead of defining and emitting separate request and response events (with this ID management) for every procedure is quite tedious and error prone, as it involves re-implementing the same functionality multiple times.

msglink avoids this by implementing the base functionality once and providing a language-specific and clean way to define procedures in one place with input data, result data and name. This is similar to [JSON-RPC](https://en.wikipedia.org/wiki/JSON-RPC) but provides the additional data validation and automatic parsing functionality described above.



## Naming alternatives

Note for future me: If msglink doesn't fit for some reason in the future, here are some alternative name ideas:

- msgio (MessageIO)


## Link collection

- wspp client reconnect: https://github.com/zaphoyd/websocketpp/issues/754#issue-353706390


## TODOS

```cpp
    // TODO: next up is moving link and event to el .hpp files. Then add some more macros, a separate event class and more
    // event define functions so a user can decide wether they want events to be just en/de codable and if they should
    // be just outgoing/incoming or both.
    // Also maybe add the possibility for an event handler as a method of the event class (maybe, not sure if so many options
    // are a good idea).
    // Then actually add this link support to the msglink server.
    // Then implement a corresponding msglink client (reconnects, ...)
    // Then implement state-management calls on a link (e.g. on_connect/on_disconnect, possibly a "currently not connected but attempting to reconnect, don't give up jet" state)
    // Then add support for data subscriptions (they need more state management (e.g. requests ) in their own classes)
    // Then add support for remote procedure calls.
```