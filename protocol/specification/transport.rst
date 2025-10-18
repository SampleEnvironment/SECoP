Transport
=========

Data transfer
-------------

SECoP relies on a stream transport of 8-bit bytes.  Most often this will be a
TCP connection.  In those cases the SEC node SHOULD support as many parallel
simultaneous connections as technically feasible.

.. important::

    SECoP does not specify a TCP port number to use for communication,
    since it is expected that in many cases, multiple SEC node servers are
    running on the same machine

"Serial port" style connections may also be used.  Here, only a single
connection can be used.  If several connections are needed, a 'multiplexer' is
needed.  This should offer multiple TCP connections and contain the necessary
logic to map requests/replies from/to those network connections onto/from the
serial connection to the actual SEC node.

Finally, SECoP messages can also be exchanged over WebSockets, which is useful
for interacting directly with browsers/JavaScript clients, see
:ref:`websockets`.


Transfer modes
--------------

Essentially the connections between an ECS and a SEC node can operate in one of
two modes:

Synchronous mode:
    where a strict request/reply pattern is used.

Async mode:
    where an update may arrive at any time (even between requests and replies).

In both cases, a request from the ECS to the SEC node must be followed by an
reply from the SEC node to the ECS, either indicating success of the request or
flag an error.

.. note::

    An ECS may try to send a request before it received the reply to an earlier
    request.  This has two implications:

    - A SEC node may serialize requests and fulfill them strictly in order.  In
      that case the ECS should not overflow the input buffer of the SEC node.
    - The second implication is that an ECS which sends multiple requests before
      the replies arrive, MUST be able to handle the replies arriving
      out-of-order.  Unfortunately there is currently no indication if a SEC
      node is operating strictly in order or if it can work on multiple requests
      simultaneously.


Operation
---------

After a connection between an ECS and a SEC node is established, the client must
verify that the SEC node is speaking a supported protocol by sending an
`identification <*IDN?>` request and checking the answer from the SEC node to
comply.  If this check fails, the connection is to be closed and an error
reported.

The second step is to query the structure of the SEC node by an exchange of
`description <describe>` messages.  After this step, the ECS knows all it needs
to know about this SEC node and can continue to either stick to a request/reply
pattern or `activate updates <activate>`.  In any case, an ECS should correctly
handle updates, even if it didn't activate them, as that may have been performed
by another client on a shared connection.


Handling timeout issues
-----------------------

If a timeout happens, it is not easy for the ECS to decide on the best strategy.
Also there are several types of timeout: idle-timeout, reply-timeout, etc.
Generally speaking: both ECS and SEC side need to be aware that the other side
may close the connection at any time.  On reconnect, it is recommended that the
ECS sends a `*IDN?` and a `describe` message.  If the responses match the
responses from the previous connection, the ECS should continue without any
internal reconfiguring, as if no interruption happened.  If the response of the
description does not match, it is up to the ECS to handle this.

Naturally, if the previous connection was activated, an `activate` message has
to be sent before it can continue as before.

.. dropdown:: Related issues

   | :issue:`004 The Timeout SEC Node Property`
   | :issue:`006 Keep Alive`


.. _websockets:

SECoP over WebSockets
---------------------

Since browser (i.e. HTML+JavaScript) based human interface solutions are more
and more important, and JavaScript lacks traditional socket based APIs,
exchanging raw SECoP messages is not an option.  The best alternative is
WebSockets (:rfc:`6455`), which are a relatively overhead-free way of
exchanging messages between two endpoints in an arbitrary pattern.

See also :ref:`rfc-007`.

Implementation in a SEC node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After opening a connection, if the first message the SEC node receives starts
with ``GET /``, it treats the connection as a WebSocket connection, i.e. it
negotiates the connection using a prelude of HTTP requests, after which the
connection continues using the WebSocket protocol in both directions.

Since WebSockets provide reliable framing, every SECoP message is sent in a
frame.  The line ending added to separate messages over raw TCP is therefore
unneeded, but remains valid.  Messages are sent as TEXT frames.

Everything else (message structure and semantics) remains unchanged.

.. note::

    If the SEC node doesn't want to support WebSockets, no further action is
    required.  It will reply with the standard SECoP error messages, and the
    client will abort the connection attempt.

    A minimal implementation of the HTTP prelude is pretty small, does not have
    a lot of complexity, and can be implemented even on microcontrollers `in
    about 200 lines of code
    <https://github.com/SampleEnvironment/microSECoP/blob/master/src/http.rs>`_.

Implementation in a client
~~~~~~~~~~~~~~~~~~~~~~~~~~

On the WebSocket client side, making a connection is as easy as opening a
connection and start sending request messages, handling response messages as
they come in.  A very minimal example in JavaScript:

.. code:: js

    function on_connect(event) {
        // On initial connect, we should ask for identification
        event.target.send('*IDN?');
    }

    function on_message(event) {
        let msg = event.data;
        // Handle response to initial *IDN? and request descriptive data
        if (msg.startsWith('ISSE')) {
            event.target.send('describe');
            return;
        }
        // Parse `msg` as a SECoP message here, and react to it
    }

    let ws = new WebSocket('ws://node:10767');
    ws.addEventListener('open', on_connect);
    ws.addEventListener('message', on_message);
    // Should also listen on 'close' and 'error' events

    // Whenever needed, send messages, for example:
    ws.send('change mod:param 42');
