Messages
========

This section specifies the structure of the messages that are transferred over a
connection with SECoP ("on the wire").  This is of interest for implementors of
new clients and/or servers, users of frameworks will find :doc:`modules` more
relevant.

A message is one line of text, coded in ASCII.  A message ends with a line feed
character (ASCII 10, ``"\n"`` in C style syntax), which may be preceded by a
carriage return character (ASCII 13, ``"\r"``), which must be ignored.

A message starts with an action keyword, followed optionally by one space and a
specifier (not containing spaces), followed optionally by one space and a JSON
value called data, which contains all remaining characters in the line.

The specifier consists of a module identifier, and for most actions, followed by
a colon as separator and an accessible identifier.  In special cases
(e.g. `describe`, `ping`), the specifier is just an arbitrary token or may be
empty.

.. dropdown:: Syntax diagrams
    :icon: code

    .. image:: images/messages.svg
       :alt: messages ::= (message CR? LF) +

    .. image:: images/message-structure.svg
       :alt: message_structure ::= action ( SPACE specifier ( SPACE data )? )?

    .. image:: images/specifier.svg
       :alt: specifier ::= module | module ":" (parameter|command)

    .. image:: images/name.svg
       :alt: name ::= [a-zA-Z_] [a-zA-Z0-9_]*

.. toctree::
    :hidden:

    messages/identification
    messages/description
    messages/activation
    messages/update
    messages/readwrite
    messages/commands
    messages/optional
    messages/heartbeat
    messages/errors

.. rubric:: Message overview

======================= ============== ==================
 Message intent          Kind           Message structure
======================= ============== ==================
 Identification          request        `*IDN?`
      \                  reply          `ISSE,SECoP,,version <ISSE,SECoP,<draft-date>,<version>>`
 Description             request        `describe`
      \                  reply          `describing . {...} <describing>`
 Activate updates        request        `activate`
      \                  reply          `active`
 Deactivate updates      request        `deactivate`
      \                  reply          `inactive`
 Heartbeat               request        `ping token <ping>`
      \                  reply          `pong token [...] <pong>`
 Read request            request        `read mod:param <read>`
      \                  reply          `reply mod:param [...] <reply>`
 Change value            request        `change mod:param value <change>`
      \                  reply          `changed mod:param [...] <changed>`
 Execute command         request        `do mod:cmd value <do>`
      \                  reply          `done mod:cmd [...] <done>`
 Value update event      event          `update mod:param [...] <update>`
 Error reply             reply          `error_action specifier [...] <error>`
 Checking (\*)           request        `check mod:param value <check>`
      \                  reply          `checked mod:param value <checked>`
 Logging (\*)            request        `logging mod level <logging>`
      \                  reply          `logging mod level <logging>`
      \                  event          `log module:level message <log>`
======================= ============== ==================

Messages marked with (\*) are optional.  A SEC node can omit their
implementation and reply as to all other unsupported messages with a
`ProtocolError`.

All other messages must be implemented.  For example, `change` is mandatory,
even if only readonly accessibles are present.  In this case, a `change` message
will naturally be replied to with an `error_change` message with an :ref:`error
class <error-classes>` of `ReadOnly` and not with a `ProtocolError`.


Data transfer
-------------

SECoP relies on a stream transport of 8-bit bytes.  Most often this will be a
TCP conection.  In those cases the SEC node SHOULD support as many parallel
simultaneous connections as technically feasible.

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
   where an update may arrive any time (even between requests and replies).

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


Messages
--------

All identifiers (for properties, accessibles and modules) are composed of ASCII
letters, digits and underscores, where a digit may not appear as the first
character.

Identifiers starting with underscore ('custom-names') are reserved for custom
extensions, e.g. messages or parameters not specified by the standard.  The
identifier length is limited (<=63 characters).

.. note::

    Although names MUST be compared/stored case sensitive, names in each scope
    need to be unique when lowercased.  The scopes are:

    - module names on a SEC node (including the group entries of those modules)
    - accessible names of a module (including the group entries of those
      parameters) - each module has its own scope
    - properties
    - names of elements in a :ref:`struct <struct>` (each struct has its own scope)
    - names of variants in an :ref:`enum <enum>` (each enum has its own scope)
    - names of qualifiers

SECoP defined names are usually lowercase, though that is not a restriction
(esp. not for module names).

A SEC node might implement custom messages for debugging purposes, which are not
part of the standard.  Custom messages start with an underscore or might just be
an empty line.  The latter might be used as a request for a help text, when
logged in from a command line client like telnet or netcat.  Messages not
starting with an underscore and not defined in the following list are reserved
for future extensions.

When implementing SEC nodes or ECS clients, a 'MUST-ignore' policy should be
applied to unknown or additional parts.  Unknown or malformed messages are to be
replied with an appropriate `ProtocolError` by a SEC node.  An ECS-client must
ignore the extra data in such messages.  See also section
:ref:`future-compatibility`.


Theory of operation:
    After a connection between an ECS and a SEC node is established, the client
    must verify that the SEC node is speaking a supported protocol by sending an
    `identification <*IDN?>` request and checking the answer from the SEC node to comply.
    If this check fails, the connection is to be closed and an error reported.

    The second step is to query the structure of the SEC node by an exchange of
    `description <describe>` messages.  After this step, the ECS knows all it needs to know
    about this SEC node and can continue to either stick to a request/reply
    pattern or `activate updates <activate>`.  In any case, an ECS should correctly handle
    updates, even if it didn't activate them, as that may have been performed by
    another client on a shared connection.


.. _message-compat:

Compatibility
-------------

This specification defines a set of requests and replies above.  Only those
messages are ALLOWED to be generated by any software complying to this
specification:

.. compound::
    Any ECS is allowed to generate the following messages:

    .. image:: images/defined-requests.svg
       :alt: defined_requests

.. compound::
    Any SEC node is allowed to generate the following messages:

    .. image:: images/defined-replies.svg
       :alt: defined_replies

The specification is intended to grow and adopt to new needs. (Related issue:
:issue:`038 Extension mechanisms`.) To futureproof the the communication, the
following messages MUST be parsed and treated correctly (i.e. the ignored_value
part is to be ignored).

.. compound::
    Any SEC node **MUST** accept the following messages and handle them properly:

    .. image:: images/must-accept-requests.svg
       :alt: must_accept_requests

.. compound::
    Any ECS **MUST** accept the following messages and handle them accordingly:

    .. image:: images/must-accept-replies.svg
       :alt: must_accept_replies

As a special case, an argumentless command may also by called without specifying
the data part.  In this case an argument of null is to be assumed.  Also, an
argumentless ping is to be handled as a ping request with an empty token string.
The corresponding reply then contains a double space.  This MUST also be parsed
correctly.

Similarly, the reports need to be handled like this:

.. _data-report:

.. compound::
    Data report:

    .. image:: images/data-report.svg
       :alt: data_report ::= "[" JSON-value "," qualifiers ("," ignored_value)* "]"

.. _error-report:

.. compound::
    Error report:

    .. image:: images/error-report.svg
       :alt: error_report ::= '["' errorclass '","' error_msg '",' error_info ("," ignored_value)* "]"

Essentially this boils down to:

1) ignore additional entries in the list-part of reports
#) ignore extra keys in the qualifiers, structure report and error report
   mappings
#) ignore message fields which are not used in the definition of the messages
   (i.e. for `describe`)
#) treat needed, but missing data as null (or an empty string, depending on
   context)
#) if a specifier contains more ":" than you can handle, use the part you
   understand, ignore the rest (i.e. treat ``activate module:parameter`` as
   ``activate module``, ignoring the ``:parameter`` part)
#) same for error class (i.e. treat ``WrongType:MustBeInt`` as `WrongType`,
   ignoring the ``:MustBeInt`` part)
#) upon parsing a value, when you know it should be one element from an
   :ref:`enum <enum>` (which SHOULD be transported as integer), if you find a string
   instead and that string is one of the names from the Enum, use that entry.
#) check newer versions of the specification and check the issues as well, as
   the above may change

Complying to these rules maximizes the possibility of future + backwards
compatibility.

.. note:: Also check :issue:`036 Dynamic units` *as it may have implications for
          a certain implementation.*


Handling Timeout Issues
~~~~~~~~~~~~~~~~~~~~~~~

If a timeout happens, it is not easy for the ECS to decide on the best strategy.
Also there are several types of timeout: idle-timeout, reply-timeout, etc...
Generally speaking: both ECS and SEC side need to be aware that the other side
may close the connection at any time!  On reconnect, it is recommended that the
ECS sends a `*IDN?` and a `describe` message.  If the responses match the
responses from the previous connection, the ECS should continue without any
internal reconfiguring, as if no interruption happened.  If the response of the
description does not match, it is up to the ECS to handle this.

Naturally, if the previous connection was activated, an `activate` message has
to be sent before it can continue as before.

.. dropdown:: Related Issues

   | :issue:`004 The Timeout SEC Node Property`
   | :issue:`006 Keep Alive`


.. _websockets:

SECoP over WebSockets
---------------------

Since browser (i.e. HTML+JavaScript) based human interface solutions are more
and more important, and JavaScript lacks traditional socket based APIs,
exchanging raw SECoP messages is not an option.  The best alternative is
WebSockets (RFC :rfc:`6455`), which are a relatively overhead-free way of
exchanging messages between two endpoints in an arbitrary pattern.

See also `SECoP RFC 7`_.

Implementation in a SEC node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After opening a connection, if the first message the SEC node receives starts
with ``GET /``, it treats the connection as a WebSocket connection, i.e. it
negotiates the connection using a prelude of HTTP requests, after which the
connection continues using the WebSocket protocol in both directions.

Since WebSockets provide reliable framing, every SECoP message is sent in a
frame.  The line ending added to separate messages over raw TCP is therefore
unneded, but remains valid.  Messages are sent as TEXT frames.

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
they come in.  A very minimal example in JavaScript::

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


.. _SECoP RFC 7: https://github.com/SampleEnvironment/SECoP/blob/master/rfcs/RFC-007-websockets.rst
