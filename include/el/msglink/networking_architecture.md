# msglink networking architecture

in addition to defining a protocol and convenient abstraction libraries for using it, msglink libraries also define a number of different networking topologies and convenient support for them in library implementations. 

This page describes the common internal architecture "stack" of msglink implementation libraries as well as different ways multiple communication parties can interact with each other over the network.


## Internal architecture

An application using the msglink protocol is divided into three blocks:

- **Network Interface**: This block is responsible for opening, maintaining and closing a network connection to the other communication party as well as sending and receiving protocol data units (here messages). It also manages and updates the other blocks higher up in the stack depending on the network connection state.
- **Link Interface**: This is a small block which is used by the link to communicate to the network interface for sending data and controlling the connection based on the protocol.
- **Link**: This block is the responsible for managing the protocol. It receives/sends and decodes/encodes messages from/to the network interface and performs actions according to the msglink protocol. This part is responsible for performing authentication with the other party as soon as a connection is established, checking protocol compatibility, managing event- and data-subscriptions, keeping track of RPC transactions and more.

## Link configuration

The user of the msglink library must customize the **link** by defining their application specific events, data subscriptions and remote procedures. They can then use the link (typically an instance of a link class) to emit events or register event handlers, call remote procedures and request/provide data to/from the remote party.

In the C++ library, this is accomplished by first defining a class/struct for each event, data subscription and remote procedure:

```cpp
// can be incoming, outgoing or both (bidirectional)
struct didi_event : public el::msglink::bidirectional_event
{
    int height;
    std::string shirt_color;

    EL_MSGLINK_DEFINE_BIDIRECTIONAL_EVENT(
        didi_event,
        height,
        shirt_color
    )
};

// ... more events and other stuff
```

Then, a custom link class has to be created which defines all of the events, data subscriptions and procedures that are part of the application specific "protocol". There, you might also configure some static listener functions called when an e.g. an event is received:

```cpp

class my_link : public el::msglink::link
{
    using link::link;   // default constructor

    // user-defined link version number that must match on both parties
    EL_MSGLINK_LINK_VERSION(1);

    // method to handle didi even
    void handle_didi(didi_event &_evt)
    {
        EL_LOGI(
            "Got inout_didi_event with height=%d and shirt_color=%s", 
            _evt.height, 
            _evt.shirt_color.c_str()
        );
    }

public:
    // object to manage event subscription
    std::shared_ptr<el::msglink::event_subscription> didi_subscription;

    // override the define method to add protocol definitions
    virtual void define() noexcept override
    {
        // define event and subscribe a listener function to it
        // (event type can be inferred from listener function parameter)
        didi_subscription = define_event(&my_link::handle_didi);
        // without listener, event needs to be named manually:
        //define_event<didi_event>();

        // ... define more events and other stuff
    }
};
```

Once the link has been defined, it can the be used when connecting with another communication party. The actual network connection part can vary greatly depending on the use case and is explained in more detail in [Networking topologies](#networking-topologies).

For the communication to work, both parties need to have a compatible link. The link is compatible if all of the below are true:

- msglink versions are compatible
- user defined link version (```EL_MSGLINK_LINK_VERSION(...)```) matches
- event, data sub. and procedure requirements match, meaning one party can provide all e.g. events the other needs (see [Authentication Procedure](msglink_protocol.md#authentication-procedure) for details)


## Networking topologies

In the context of this document, the them "networking topology" refers to the relation between client and server parties in a msglink communication session.

At it's core, msglink is a point-to-point communication protocol. msglink never uses broadcasts or multicasts on a network level. There are always two parties A and B communication with each other using the msglink protocol.

In the msglink protocol, there is no logical difference between a client and a server. Both communication parties are equal and have equal capabilities (in the bounds defined by the user application).

However, at some point, a network connection (Websocket, which builds ontop of TCP socket) needs to be established as a communication channel. For that to work, one of the two parties needs to act as a server, listening for a new connection by the other party. It is the job of the **Network Interface** architecture block to manage this connection phase. This network interface block is typically represented by a class. There are multiple network interface blocks that the user of the library can select from to assign a communication party the appropriate role in the connection establishing phase.

### Networking role selection

Depending on the application, the user may arbitrarily define either party to be the server or the client by using the corresponding network interface class ```el::msglink::server``` or ```el::msglink::client```.

In many cases, like a WebApp, the role selection is obvious. In this example, it only makes sense for a program running on the webserver with a public facing IP address to be the msglink server and the browser to be the client.<br>
In most applications, one party is clearly the "boss" of operations and it therefor makes sense for that one to be the server.

In a scenario two equivalent devices such as two robots on the same network communicating directly with each other, it might not matter which one is the server. When selecting the before mentioned interface classes, there is only a minimal difference in the library API.

### Connection loss and reconnects

In msglink, the protocol an communication state is managed by the **Link** class, while the network connection state is managed by the **Network Interface** class such as ```el::msglink::server``` or ```el::msglink::client```. When a connection is first established, the network interface instance notifies the link instance and it can perform authentication. Once that is done, the two parties might exchange event subscriptions and start communicating. 

But what happens when the connection is lost? msglink intentionally doesn't define any convoluted method for attempting to restore a lost connection and resume communication where it left off. When the connection is closed, it's closed for good and must be restarted from scratch. 

Since the communication is not resumed where it left off, we might as well delete the link instance entirely. As soon as the client detects the connection has closed, it can attempt to reconnect to the server the same way as before. A new link instance would be created and the authentication procedure repeated the same way. But what happens to the event subscriptions and event listeners present before the connection loss? They would just be forgotten entirely and the user application would need to make sure what was required and what listener functions were registered in order to re-subscribe to all the events. This would be very tedious for the user to implement.

For this reason, in a simple application were there is only one client and server which need to communicate, the **Link** block is completely decoupled from the **Network Interface** block. For the entire lifetime of an application, there will be a single instance of the user-defined link class that is never deleted. The network interface block will simply receive a reference to this instance to control it. Internally in the link class, the user protocol state (subscriptions, ...) is also decoupled from the network and msglink protocol state (authentication, transactions, ...).

Initially, the link instance is in a disconnected state, were the network interface block has not jet created a connection to the other party. During this time, other parts of the application can already access the link instance and register event listeners or emit events. They will not notice the connection isn't established jet and the link will keep track internally of which events are subscribed and may queue up emitted events (depending on configuration)
As soon as a connection is established and the authentication process is complete, the link will send all the event subscriptions for the currently required events according to it's internal records as well as possibly pushing queued event emissions. 

Any event subscriptions and other state changes made while the connection is established are immediately sent to the other party as well as recorded internally, so the link instance knows at all times what event subscriptions, data subscriptions, etc. it currently needs.

When the connection is lost, both parties' network connection blocks will reset the link instance' msglink protocol and networking state to a time before authentication. This does not affect user protocol state such as event subscriptions. Form then on, the link will behave like it did before the connection was established. Any access by the rest of the application will be recorded so the current state and missed events can be sent to the other party as soon it reconnects, ensuring the state of the communication subsystem will always be clean and the communication channel behaves as the user application expects.


### many-to-one relationship

In many cases, such as a server for a WebApp, it might be required for a server application to handle connection to multiple independent clients at the same time, dynamically. This is called a many-to-ony relationship. To support this functionality, you can use a special network interface class instead of teh basic server: ```el::msglink::multi_connection_server```. This class dynamically creates link instances for each client that connects to the server as well as calling a "client connected" handler to perform some initialization for the new client.

This poses a problem when it comes to client state management however. As explained before, the user protocol state is kept within the link instance. In the simple server application only only exactly one connection, there was one global link instance. A server handling many connections needs to initialize links dynamically as soon as a client connects. 

So when a connection is lost, what happens to the link instance? Surely it cannot be kept around forever. Since there are many clients, it is tricky to tell wether a client is trying to reconnect or it's simply a new client connecting, which makes it hard to match a new connection to an existing link. That would require some sort of session identifier, which is currently not supported by msglink (more on that later).

Luckily we can work around this issue in many cases by changing the design philosophy of the application or just viewing it differently.
When there is a server responsible for *serving* a large number of clients, it is unlikely for the server to *desperately want to get some specific piece of information globally* from *a* specific client. Instead, in most cases the clients probably want's to get notified about events from the server, so the server might not even have any event subscriptions for the client. If the client want's the server to do something, an RPC is probably better suited. And for the cases were client events are listened to by the server, every client probably has it's own event listeners associated to it, which are registered as soon as the link is instantiated when the client connects (probably methods of the link itself). In most cases the server isn't going to say *I now temporarily need to be notified about this event from the client* during runtime. And once a client disconnects, the server probably doesn't care about a specific event from that client, as it is the client which *wants* to send the information to the server.

### Link lifetime

For all these reasons, the ```el::msglink::multi_connection_server``` doesn't reconnect clients to existing links. The lifetime of a link is from when the connection is first established to when the connection is closed for whatever reason.

The simple ```el::msglink::server``` class and also the ```el::msglink::client``` class keep the link around for the lifetime of the application, meaning the link is created when the application launches and deleted when it exits. Since these only have one global link instance, they reconnect it to the connection after the connection is closed.

> ### Note
> The ```el::msglink::multi_connection_server``` is made for multiple ```el::msglink::client```s to connect to it. Even though the server-side link is not persistent, the client still only has one global link instance which behaves as described in the single-connection example. So when a client looses connection to a multi connection server, it still keeps and re-activates it's subscriptions just as expected.

In the future it might be possible to add support for some sort of session identifier to the msglink authentication procedure that the client (which has a persistent link) remembers after the initial connect. When the multi connection server detects a connection loss without the connection being properly closed beforehand, it can keep it's link instance alive and reconnect it to the next new connection that authenticates with the same session identifier.
