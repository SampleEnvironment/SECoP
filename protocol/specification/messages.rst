Messages
========

A message is one line of text, coded in ASCII.  A message ends with a line feed
character (ASCII 10, ``"\n"`` in C style syntax), which may be preceded by a
carriage return character (ASCII 13, ``"\r"``), which must be ignored.

A message starts with an action keyword, followed optionally by one space and a
specifier (not containing spaces), followed optionally by one space and a JSON
value (see :rfc:`8259`) called data, which contains all remaining characters in
the line.

The specifier consists of a module identifier, and for most actions, followed by
a colon as separator and an accessible identifier.  In special cases
(e.g. `describe`, `ping`), the specifier is just an arbitrary token or may be
empty.

Examples of messages are::

    > describe
    > read temp:setpoint
    > change temp:target 15.0
    < update temp:value [15.3, {"t": 1736239123.0}]

The ``<`` or ``>`` marks whether messages are sent to or received from the node.

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
      \                  reply          `describing . structure-report <describing>`
 Activate updates        request        `activate`
      \                  reply          `active`
 Deactivate updates      request        `deactivate`
      \                  reply          `inactive`
 Heartbeat               request        `ping token <ping>`
      \                  reply          `pong token data-report <pong>`
 Read request            request        `read mod:param <read>`
      \                  reply          `reply mod:param data-report <reply>`
 Change value            request        `change mod:param value <change>`
      \                  reply          `changed mod:param data-report <changed>`
 Execute command         request        `do mod:cmd value <do>`
      \                  reply          `done mod:cmd data-report <done>`
 Value update event      event          `update mod:param data-report <update>`
 Error reply             reply          `error_action specifier error-report <error>`
 Checking (\*)           request        `check mod:param value <check>`
      \                  reply          `checked mod:param data-report <checked>`
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


.. _naming:

Naming conventions
------------------

All identifiers (for properties, accessibles and modules) are composed of ASCII
letters, digits and underscores, where a digit may not appear as the first
character.

Identifiers starting with underscore ("custom names") are reserved for custom
extensions, e.g. messages or parameters not specified by the standard.  The
identifier length is limited (<= 63 characters).

.. admonition:: Case sensitivity

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

Custom messages
---------------

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
