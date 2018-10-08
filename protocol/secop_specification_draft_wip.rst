.. raw:: html

   <h1>THIS DOCUMENT IS WORK IN PROGRESS AND MAY CONTAIN HORRIBLE BUGS!</h1>

SECoP: Sample Environment Communication Protocol
################################################

V2018-10-04

Introduction
============

The main goal of the "committee for the standardization of sample
environment communication" is to establish a common standard protocol
SECoP for interfacing sample environment equipment to experiment control
software.

  Definition: Experiment Control Software ECS
     Software controlling the hardware for carrying out an experiment. Includes the user
     interface. Usually speaks several protocols with different parts of the instrument.
     Often also called instrument control.

There is a task (7.1) within the European framework SINE2020 also
dealing with this subject. In its description we read:

    ... The standard should be defined in a way that it is compatible
    with a broad variety of soft- and hardware operated at the different
    large scale facilities. … The adoption of this standard will greatly
    facilitate the installation of new equipment and the share of
    equipment between the facilities. ...

This does also cover the aims of the committee.

The idea is, that a sample environment apparatus can easily be moved
between facilities and instruments/beamlines. As long as the facilities
have implemented a SECoP client within its ECS, and on the apparatus a
SECoP server is implemented as the SEC node, using the apparatus for an
experiment should be straightforward. An ECS can be built in a way, that
the configuration of a SEC node may be as short as entering a network
address, as the description can be loaded over the protocol (in
addition, the electronic communication link, electricity and may be
cooling water or pressurized air etc. have to be connected, but that is
out of scope …)

  Definition: Sample Environment Control Node (SEC node)
    Computing unit or process or task, connected to all control units (temperature controller, flow controller, pressure sensor ...) of a sample environment, bridge to the ECS. SECoP specifies how ECS speaks with the SEC node.
    The SEC node allows the ECS to access a set of modules (and their parameters) via the SECoP. It also provides a list of accessible modules and parameters.

Other requirements
------------------

-  the protocol should be easy to use

-  it should be easy to implement in connection with existing ECSs and
   sample environment control software

-  it should be possible to be implemented on the most common platforms
   (operating systems and programming languages)

-  the protocol should be defined in way that allows a maximum
   **compatibility**: Newer and older versions of the syntax should
   be compatible

-  the protocol should be defined in a way, which allows a maximum
   **flexibility**: A simple (= with minimal features) ECS
   implementation should be able to communicate with a complex SEC
   node (with a lot of features), and an ECS with a rich number of
   features should be able to cope with a simple SEC node,
   implementing only a minimum number of features

.. sectnum::
    :start: 0
    :depth: 3

.. contents:: Contents
    :depth: 1
    :backlinks: entry


Hardware Abstraction
====================

.. contents::
    :local:
    :depth: 1
    :backlinks: entry


Modules
-------

  Definition: Module
    One logical component of an abstract view of the sample environment. Can at least be read.
    May be ’driven' (set new setpoint). May have parameters influencing how it achieves
    its function (e.g. PID parameters). May have some additional diagnostics (read-only) parameters.
    May provide some additional status information (temperature stable?, setpoint reached?)
    Reading a module returns the result of the corresponding physical measurement.

In earlier discussion we used the term "device" for module, which might
be misleading, as "device" is often used for an entire apparatus, like a
cryomagnet or humidity cell. In the context of SECoP, an apparatus in
general is composed of several modules. For example different
temperature sensors in one apparatus can be seen as different modules.

An SEC node controls several (or one or no) modules. Modules also have
some descriptive data (name, type, list\_of\_parameters,
list-of\_commands,...).

Parameters
----------

A module has several parameters associated with it. A parameter is
addressed by the combination of module and parameter name. Module names
have to be unique within an SEC node, parameter names have to be unique
within a module.

Module and parameter names should be in english (incl. acronyms), using
only ascii letters and some additional characters (see section "Message
Syntax"). A maximum name length might be imposed.

  Definition: Parameter
    The main parameter of a module is its value. Writable parameters may influence the
    measurement (like PIDs). Additional parameters may give more information about its
    state (running, target reached), or details about its functioning (heater power) for
    diagnostics purposes. Parameters with a predefined meaning are listed in the standard,
    they must always be used in the same way. Custom parameters are defined by the
    implementation of the SEC node, the ECS can use them only in a general way, as their
    meaning is not known.


The following parameters are predefined (extensible):

-  **value**

-  **status** (a tuple of two elements: a status with predefined values
   from an Enum as "idle","busy","error", and a describing text).
   *Remark: it is proposed to add additional states (starting,
   started, pausing, paused, stopping, warning). It has to be
   discussed, if this (and therefore a start and pause command)
   makes sense. Generally we want to keep the number of states as
   small as possible here.*

-  **target** (not present, if the module is not writable)

-  **pollinterval** (double, a hint to the module for the polling interval in seconds)

The following parameters were discussed at a meeting.

-  **ramp** (writable parameter, desired ramp. Units: main units/min)

-  **use\_ramp** (writable, 1 means: use given ramp, 0 means: go as fast as possible)

-  **setpoint** (ramping setpoint, read only)

-  **time\_to\_target** (read only, expected time to reach target)


Commands
--------

A module may also have commands associated with it. A command is
addressed by the combination of module and parameter name. Like
parameters, command names have to be unique within a module, and should
be in english (incl. acronyms), using only ascii letters and some
additional characters (see section "Message Syntax"). A maximum name
length might be imposed.

  Definition: Command
    Commands are provided to initiate specified actions of the module.
    They should return immediately after that action is initiated, i.e.
    should not wait until some other state is reached. Commands may
    need an possibly structured argument and may return a possibly structured result.
    Commands with a predefined meaning are listed in the standard,
    they must always be used in the same way.

Custom commands are defined by the implementation of the SEC node, the
ECS can use them only in a general way, as their meaning is not known.

So far the only command defined (for driveable modules) is ‘stop’ (no
argument, no result). When a modules target is changed, it is 'driving'
to a new value until the target is reached or until its stop command
is sent.
It is still to be discussed, what this exactly means for temperature
devices (heater off vs. ‘stay at current temp’).

The following commands are predefined (extensible):

-  **stop** mandatory command on a drivable. Cease movement, set the target parameter
   to a value close to the present one. Act as if this value would have been the initial target.

The following commands are foreseen, but ae not 100% fixed yet:

-  **go** optional on a drivable. If present, the 'go' command is used to start the
   module. If not present the module is started upon a change on the target
   parameter.

-  **hold** optional command on a drivable. Stay more or less where you are, cease
   movement, be ready to continue soon, target value is kept. Continuation can be
   trigger with 'go', or if not present, by putting the target parameter to its
   present value.

-  **abort** optional command. Stops the running module in a safe way (for example
   switches the heater off).

-  **reset** optional command for putting the module to a state predefined by the implementation.

-  **shutdown** optional command for shuting down the hardware.
   When this command is sent, and the triggered action is finished (status in idle mode),
   it is safe to switch off the related device.

   *remark: there is an alternative proposal for
   implementing the shutdown function, see* `SECoP Issue 22: Enable Module instead of Shutdown Command`_

   *remark: The mechanics for buffering values and the semantics for the above commands except ``stop``
   are not yet finalised. see also discussion in* `SECoP Issue 28: Clarify buffering mechanism`_

Properties
----------

  Definition: Properties
    The static information about parameters, modules and SEC nodes is
    constructed from properties with predefined names and meanings.

For a list of properties see `Descriptive Data`_.

Data report
-----------
A JSON array with the value of a parameter as its first element,
and an JSON object containing the Qualifiers_ for this value as its second element.

*remark: future revisions may append additional elements.
These are to be ignored for implementations of the current specification*

Error report
------------
An error report is only used in a `error reply`_ indicating that the requested action could
not be performed as request or that other problems occured.
The Error report is a JSON-array containing the request message leading to the report error
(minus line endings) as a string in its first element, a (short) human readable text
as its second element. The third element is a JSON-Object, containing possibly
implementation specific information about the error (stack dump etc.).

*note: errors can only be report 'for' a request. They contain a copy of the request,
so that a client may sort out, which of the requests it sent got an error.*

*remark: There is no way for a SEC-node the report some general error information without
a client sending a request.*

Structure report
----------------
The descriptive report is a structured JSON construct describing the name of modules exported
and their parameters, together with the corresponding properties.
For details see `descriptive data`_.


Qualifiers
----------

Qualifiers optionally augment the value in a reply from the SEC-Node,
and present variable information about that parameter.
They are collected as named values in a JSON-object.

Currently 2 qualifiers are defined:

- "t": (short for timestamp)
   The time when the parameter has changed or was verified/measured (when no timestamp
   is given, the ECS may use the arrival time of the update message as
   the timestamp).
   It SHOULD be given, if the SEC-node has a synchronized time,
   the format is fractional seconds since 1970-01-01T00:00:00+00:00,
   represented as a number, in general a floating point when the resolution
   is better than 1 second.

  *See also* `SECoP Issue 3:Timestamp Format`_

- "e": the uncertainity of the quantity. MUST be in the same units
   as the value. rarely used as interpretation what e means differs.
   (sigma vs. RMS difference vs. ....)

other qualifiers might be added later to the standard.
If an unknown element is encountered, it is to be ignored (for now).

*See also:* `SECoP Issue 28: Clarify buffering mechanism`_


Interface Classes
-----------------

The idea is, that the ECS can determine the functionality of a module
from its class.

Base classes:

-  Readable (has at least a value and a status parameter)

-  Writable (must have a target parameter)

-  Drivable (a Writable, must have a stop command, the status parameter will indicate
   busy for a longer-lasting operation)

For examples of interface classes see the separate document "Interface Classes and Features".
*Note: these examples are not yet part of the standard*

The standard contains a list of classes, and a specification of the
functionality for each of them. The list might be extended over time.
Already specified base classes may be extended in later releases of the
specification, but earlier definitions will stay intact, i.e. no
removals or redefinitions will occur.

The module class is in fact a list of classes (highest level class
first). The ECS chooses the first class from the list which is known to
it. The last one in the list must be one of the base classes listed above.

*remark: The list may also be empty, indicating that the module in question does not even conform to the Readable class!*

Features
--------

*Note: this is not yet part of the standard*

As the list of interface classes would risk to increase a lot with possible
combinations, *features* come into place. A feature is a modular functionality,
with some predefined parameters and commands.

For examples of features see the separate document "Interface Classes and Features".

Protocol
========

.. contents::
    :depth: 1
    :local:
    :backlinks: entry


The basic element of the protocol are messages.

Message Syntax
--------------
The byte stream which is exchanged via a connection is split into messages:

.. image:: images/messages.png
   :alt: messages ::= (message CR? LF) +

A message is essentially one line of text, coded in ASCII (may be extended to UTF-8
later if needed). A message ends with a line feed character (ASCII 10), which may be preceded
by a carriage return character (ASCII 13), which must be ignored.

.. note:: `␣` is used instead of the SPACE character (%x20) for better visibility in the following diagrams.*


All messages share the same basic structure:

.. image:: images/message_structure.png
   :alt: message_structure ::= action ( SPACE specifier ( SPACE data )? )?

i.e. message starts with an action keyword, followed optionally by one space and a specifier
(not containing spaces), followed optionally by one space and a JSON-text
formatted value (see :RFC:`8259`) called data.

.. Note:: numerical values and strings appear 'naturally' formatted in JSON-text, i.e. 5.0 or "a string".

The specifier consists of a module identifier and for most actions followed by a colon as separator
and a parameter or command identifier:

.. image:: images/specifier.png
   :alt: specifier ::= module | module ":" (parameter|command)

The identifiers are composed by
ascii letters, digits and underscore, where a digit may not
appear as the first character.

.. image:: images/name.png
   :alt: name ::= [a-zA-Z_] [a-zA-Z0-9_]*

Identifiers starting with underscore are
reserved for special purposes like internal use for debugging. The
identifier length is limited (<=63 characters). Module names on a SEC Node
and parameter names within a module must not differ when uppercase letters
are replaced by their lowercase counterparts, i.e. though names may contain uppercase letters,
they need to be unique, when lowercased.

A SEC node might implement custom messages for debugging purposes, which are not
part of the standard. Custom messages start with an underscore or might just be
an empty line. The latter might be used as a request for a help text, when logged
in from a command line client like telnet or netcat. Messages not starting with
an underscore and not defined in the following list are reserved for future extensions.

When implementing SEC-nodes or ECS-clients, a 'MUST-ignore' policy should be applied to unknown or additional
datafields. Unknown messages are to be replied with an appropriate ProtocolError by a SEC-Node.
An ECS-client must ignore such messages. See also section `Future Compatibility`_.

.. table::

    ======================= ============== ==================
     message intent          message kind   message elements
    ======================= ============== ==================
     `identification`_       request        ``*IDN?``
          \                  reply          ISSE&SINE2020\ **,SECoP,**\ *version,add.info*
     `description`_          request        ``describe``
          \                  reply          ``describing .`` <`Structure Report`_>
     `activate updates`_     request        ``activate [module]``
          \                  reply          ``active [module]``
     `deactivate updates`_   request        ``deactivate [module]``
          \                  reply          ``inactive [module]``
     `heartbeat`_            request        ``ping id``
          \                  reply          ``pong id`` <`Data Report`_>
     `change value`_         request        ``change module:parameter value``
          \                  reply          ``changed module:parameter`` <`Data Report`_>
     `execute command`_      request        ``do module:command`` <argument or null>
          \                  reply          ``done module:command`` <`Data Report`_>
     `read request`_         request        ``read module:parameter``
     value update_  event    async. msg.    ``update module:parameter`` <`Data Report`_>
     `error reply`_          reply          ``error errorclass`` <`Error Report`_>
    ======================= ============== ==================

*Remark: We tried to keep this list small. However a possible extension is discussed in*
`SECoP Issue 29: New messages for buffering`_

Message intents
---------------

Identification
~~~~~~~~~~~~~~

The syntax of the identification message differs a little bit from other
messages, as it should be compatible with IEEE 488.2. The identification
request "\ **\*IDN?**\ " is meant to be sent as the first message after
establishing a connection. The reply consists of 4 comma separated
fields, where the second and third field determine the used protocol.

In this and in the following examples, messages sent to the server are marked with "> ",
and messages sent to the client are marked with "< "

Example:

.. code::

  > *IDN?
  < ISSE&SINE2020,SECoP,V2018-10-04,draft

Description
~~~~~~~~~~~

The next messages normally exchanged are the description request and
reply. The reply contains the `Structure report`_ i.e. a structured JSON object describing the name of
modules exported and their parameters, together with the corresponding
properties.

Example:

.. code::

  > describe
  < describing . {"modules":["t1",["class":[ "temperature\_sensor","readable"],"parameters":["value", ...

The dot (second item in the reply message) is a placeholder for extensibility reasons.
A client implementing the current specification may savely ignore it.

*Remark:
this reply might be a very long line, no line breaks are allowed in the
JSON value!*

Activate Updates
~~~~~~~~~~~~~~~~

The parameterless "activate" request triggers the SEC node to send the
values of all its modules and parameters as update messages. When this
is finished, the SEC node must send an "active" reply. (*global activation*)

A SEC node might accept a module name as second item of the
message, activating only updates on the parameters of the selected module.
In this case, the "active" reply also contains the module name. (*module-wise activation*)

A SEC Node not implementing module-wise activation MUST NOT sent the module
name in its reply, and MUST activate all modules (*fallback mode*).

*remark: This mechanism may be extended to specify modulename:parametername for a parameter-wise activation.
A SEC-node capable of module-wise activation SHOULD NOT fallback to global activation
if it encounters such a request. Instead it SHOULD fallback to module-wise activation,
i.e. ignore anything after (including the) colon in the specifier.*

Update
~~~~~~

When activated, update messages are delivered without explicit request
from the client. The value is a `Data report`_, i.e. a JSON array with the value as its first
element, and an JSON object containing the `Qualifiers`_ as its second element.

Example:

.. code::

  > activate
  < update t1:value [295.13,{"t":1505396348.188388,"e":0.01}]
  < update t1:status [[400,"heater broken or disconnected"],{"t":1505396348.288388}]
  < active

Deactivate Updates
~~~~~~~~~~~~~~~~~~

A parameterless message. After the "inactive" reply no more updates are
delivered if not triggered by a read message.

Example:

.. code::

  > deactivate
  < update t1:value [295.13,{"t":1505396348.188388}]
  < inactive

*remark: the update message in the second line was sent before the deactivate message
was treated. After the "inactive" message, the client can expect that no more untriggered
update message are sent.*

The deactivate message might optionally accept a module name as second item
of the message for module-wise deactivation. If module-wise deactivation is not
supported, it should ignore a deactivate message which contains a module name.

*Remark: it is not clear, if module-wise deactivation is really useful. A SEC Node
supporting module-wise activation does not necessarily need to support module-wise
deactivation.*

Change Value
~~~~~~~~~~~~

the change value message contains the name of the module or parameter
and the value to be set. The value is JSON formatted, but note that for
a floating point value this is a simple decimal coded ASCII number. As
soon as the set-value is read back from the hardware, all clients having activated
the parameter/module in question get an "update" message is sent.
After all side-effects are communicated, a "changed" reply is then send, containing a
`Data report`_ of the read-back value.

*remark: If the value is not stored in hardware, the "update" message can be sent immediately.*

*remark: The read-back value should always reflect the value actually used.*

Example on a connection with activated updates. Qualifiers are replaced by {...} for brevity here.

.. code::

  > read mf:status
  < update mf:status [[100,"OK"],{...}]
  < change mf:target 12
  < update mf:status [[300,"ramping field"],{...}]
  < changed mf:target [12,{...}]

The status changes from "idle" to "busy". The ECS will be informed with a further update message on mf:status, when the module has finished ramping.

**note:** it is vital that all 'side-effects' are realised (i.e. stored in internal variables) and be communicated, **before** the 'changed' reply is sent!

Read Request
~~~~~~~~~~~~

With the read request message the ECS may ask the SEC node to update a
value as soon as possible, without waiting for the next regular update.
The reply is an update message. If updates are not activated, the
message can be treated like a read message in a request-reply scheme as
in the previous SECoP proposal.

Example:

.. code::

  > read t1:value
  < update t1:value [295.13,{"t":1505396348.188}]
  > read t1:status
  > update t1:status [[100,"OK"],{"t":1505396348.548}]

*remark: If a client has activated the module/parameter for which it sent a ``read`` request,
it may receive more than one 'update' message, especially if SEC-node side polling is active.
There is no indication, which message was sent due to polling (or other clients requesting a 'read')
and or due to a specific read. An ECS-client may just use the first matching message and treat it
as 'the reply'.*

_`Execute Command`
~~~~~~~~~~~~~~~~~~

If a command is specified with a single argument, the actual argument is given in
the data part as a json-text. This may be also a json-object if the datatype of
the argument specifies that
(i.e. the type of the single argument can also be a struct, tuple or an array, see `data types`_).
The types of arguments must conform to the declared datatypes from the datatype of the command argument.

A command may also have a return value, which may also be structured.
The "done" reply always contains a `Data report`_ with the return value.
If no value is returned, the data part is set to "null".
The "done" message should be returned quickly, the time scale should be in the
order of the time needed for communications. Still, all side-effects need to be realised
and communicated before.
Actions which have to wait for physical changes, can be triggered with a command, but not be waited upon.
The information about the duration and success of such an action has to be transferred via the status parameter.

*remark: If a command does not required an argument,
the argument SHOULD be transferred as json-null.
A SEC-Node SHOULD also accept the message, if the data part is emtpy and perform the same action.*

Example:

.. code::

  > do t1:stop
  < done t1:stop [null, {"t": 1505396348.876}]

  > do t1:stop null
  < done t1:stop [null, {"t": 1505396349.743}]

Error Reply
~~~~~~~~~~~

Contains an error class from the list below as its second item.
The third item of the message is an `Error report`_, containing the request message
(minus line endings) as a string in its first element, a (short) human readable text
as its second element. The third element is a JSON-Object, containing possibly
implementation specific information about the error (stack dump etc.).

Example:

.. code::

  > read tx:target
  < error NoSuchModule ["read tx:target", "tx is not configured on this SEC node", {}]
  > read ts:target
  < error NoSuchParameter ["read ts:target", "ts has no parameter target", {}]
  > meas:volt?
  < error SyntaxError ["meas:volt?", "unknown keyword", {}]

Error Classes

.. list-table::
    :widths: 20 80

    * - NoSuchModule
      - The action can not be performed as the specified module is non-existent.

    * - NoSuchParameter
      - The action can not be performed as the specified parameter is non-existent.

    * - NoSuchCommand
      - The specified command does not exist.

    * - CommandFailed
      - The command failed to execute.

    * - CommandRunning
      - The command is already executing.

    * - ReadOnly
      - The requested write can not be performed on a readonly value..

    * - BadValue
      - The requested write or Command can not be performed as the value is malformed or of wrong type.

    * - CommunicationFailed
      - Some communication (with hardware controlled by this SEC-Node) failed.

    * - IsBusy
      - The reequested write can not be performed while the Module is Busy

    * - IsError
      - The requested action can not be performed while the module is in error state.

    * - Disabled
      - The requested action can not be performed at the moment. (Interlocks?)

    * - SyntaxError
      - A malformed Request or on unspecified message was sent

    * - InternalError
      - Something that should never happen just happened.

*remark: This list may be extended, if needed. clients should treat unknown error classes as generic as possible.*


Heartbeat
~~~~~~~~~
In order to detect that the other end of the communication is not dead,
a heartbeat may be sent. The second part of the message (the id) must
not contain a space and should be short and not be re-used.
It may be omitted. The reply will contain exactly the same id.

A SEC-node replies with a ``pong`` message with a `Data report`_ of a null value.
The `Qualifiers`_ part SHOULD only contain the timestamp (as member "t") if the
SEC-node support timestamping.
This can be used to synchronize the time between ECS and SEC-node.
*remark: The qualifiers could also be an empty JSON-object*

For debugging purposes, when *id* in the ``ping`` request is omitted,
in the ``pong`` reply there are two spaces after ``pong``.
A client SHOULD always send an id. However, the client parser MUST treat two
consecutive spaces as two separators with an empty string in between.

Example:

.. code::

  > ping 123
  < pong 123 [null, {"t": 1505396348.543}]


*Related SECoP Issues:* `SECoP Issue 3:Timestamp Format`_ and `SECoP Issue 7:Time Synchronization`_



Timeout Issues
~~~~~~~~~~~~~~

If a timeout happens, it is not easy for the ECS to decide on the
best strategy.
Generally speaking: both ECS and SEC side needs to be aware that the other
side may close the connection at any time! On reconnect, it is recommended,
that the ECS does send a \*IDN? and a describe message. If the reponses match
the responses from the previous connection, the ECS should continue
as if no interruption happend. Of course, if the connection was previously activated,
it needs to be activated again.
If the response of the description does not
match, it is up to the ECS how to handle this.
Naturally, if the previous connection was in asynchronous mode, an activate
message has to be sent before it can continue as before.

*Related SECoP Issues:* `SECoP Issue 4: The Timeout SEC Node Property`_ and `SECoP Issue 6: Keep Alive`_


Multiple Connections
--------------------

A SEC node may accept only a limited number of connections, downto 1.
However, each SEC node should support as many connections as technically
feasible.

Details about how to multiplex multiple connections onto one are to be
discussed.


Descriptive Data
================

.. contents::
    :depth: 1
    :local:
    :backlinks: entry

Format of Descriptive Data
--------------------------

The format of the descriptive data is JSON, as all other data in SECoP.


.. for creating the railroad diagrams see: http://bottlecaps.de/rr/ui
.. source EBNF:
.. SEC_node_description ::= '{' (SEC_node_property ( ',' SEC_node_property)* )? '}'
.. SEC_node_property ::= property |  ( '"modules":' '[' (name ',' module_description (',' name ',' module_description)*)? ']')
.. module_description ::= '{' (module_property ( ',' module_property)* )? '}'
.. module_property ::= property |  ( '"parameters":' '[' (name ',' properties (',' name ',' properties)*)? ']') |  ( '"commands":' '[' (name ',' properties (',' name ',' properties)*)? ']')
.. properties ::=  '{' (property ( ',' property)* )? '}'
.. property ::= (name ':' property_value)

SEC node description
~~~~~~~~~~~~~~~~~~~~

.. image:: images/sec_node_description.png
   :alt: SEC_node_description ::= '{' (SEC_node_property ( ',' SEC_node_property)* )? '}'

SEC node property
~~~~~~~~~~~~~~~~~

.. image:: images/sec_node_property.png
   :alt: SEC_node_property ::= property |  ( '"modules":' '[' (name ',' module_description (',' name ',' module_description)*)? ']')

module description
~~~~~~~~~~~~~~~~~~

.. image:: images/module_description.png
   :alt: module_description ::= '{' (module_property ( ',' module_property)* )? '}'

module property
~~~~~~~~~~~~~~~

.. image:: images/module_property_v2.png
   :alt: module_property ::= property |  ( '"accessibles":' '[' (name ',' properties (',' name ',' properties)*)? ']') ']')

properties
~~~~~~~~~~

.. image:: images/properties.png
   :alt: properties ::=  '{' (property ( ',' property)* )? '}'

property
~~~~~~~~

.. image:: images/property.png
   :alt: property ::= (name ':' property_value)


SEC Node Properties
-------------------

there might be properties such as a timeout which are relevant for the
communication of a SEC node.

-  **equipment_id** a worldwide unqiue id of an equipment as string. Should contain the name of the
   owner institute or provider company as prefix in order to guarantee worldwide uniqueness.

   example: ``"MLZ_ccr12"`` or ``"HZB-vm4"``

-  **description** (mandatory, a text describing the node, in general, the first
   line is a short description (line break \\n))

   the formatting should follow the 'git' standard, i.e. a short headline (max 72 chars),
   followed by \n\n and then a more detailed description.

-  **firmware** (optional, a short string naming the version of the SEC node software)

   example: ``frappy-0.6.0``

-  **timeout** (optional, value in seconds, a SEC node should be able to respond within
   a time well below this value. Default: 10 sec, *see* `SECoP Issue 4: The Timeout SEC Node Property`_)


Module Properties
-----------------

-  **description** (mandatory) a text describing the module, formatted like the node-property description

-  **visibility** (optional: 3=expert, 2=advanced, 1=user (default)), Note: this
   does not imply that the access is controlled. It may just be a
   hint to the UI for the amount of exposed modules. A visibility of 2 means
   that the UI should hide the module for users, but show it for experts and
   advanced users.

-  **interface\_class** (mandatory) a list of classes for the module, for example
   ["Magnet", "Drivable"]

-  **features** (optional) (a list of features for the module, for example
   ["HasRamp", "HasTolerance"]),
   *this is not yet part of the standard, see also:* `SECoP Issue 18: Interface classes`_)

-  **group** (optional identifier, may contain ':' which may be interpreted as path separator)
   The ECS may group the modules according to this property.
   The lowercase version of a group must not match any lowercase version of a module name on
   the same SEC node. (*see:* `SECoP Issue 8: Groups and Hierarchy`_)

-  **meaning** (optional) a module property with a tuple as its value, with the following two elements:

   1. a string from an extensible list of predefined meanings:

      * 'temperature'   (the sample temperature)
      * 'temperature_regulation' (to be specified only if different from 'temperature')
      * 'magneticfield'
      * 'electricfield'
      * 'pressure'
      * 'rotation_z' (counter clockwise when looked at 'from sky to earth')
      * 'humidity'
      * 'viscosity'
      * 'flowrate'
      * 'concentration'

      This list can be extended later. (*see:* `SECoP Issue 26: More Module Meanings`_).

      '_regulation' can be postfixed, if the quantity generating module is different from the
      (closer to the sample) relevant measuring device. A regulation device MUST have an
      ``interface_class`` of at least ``Writable``.

   2. a value describing the importance, with the following values:

      - 10 means the instrument/beamline (Example: room temperature sensor always present)
      - 20 means the surrounding sample environemnt (Example: VTI temperature)
      - 30 means an insert (Example: sample stick of dilution insert)
      - 40 means an addon added to an insert (Example: a device mounted inside a dilution insert)

      Intermediate values might be used. The range for each category starts at the indicated value minus 5
      and ends below the indicated value plus 5. (*see also:* `SECoP Issue 9: Module Meaning`_)

Parameter Properties
--------------------

-  **description** (mandatory) a text describing the parameter, formatted as for module-description
   or node-description

-  **readonly** (mandatory), a boolean value indiciation wheater this parameter may be changed, or not

-  **datatype** (mandatory) datatype of the parameter, see `Data Types`_

-  **unit** (default: unitless, should be given, if meaningfull, empty string: unit is one)
   Only SI-units (including prefix) SHOULD be used for SECoP units preferrably.

-  **visibility** (optional: 3=expert, 2=advanced, 1=user (default)), Note: this
   does not imply that the access is controlled. It may just be a
   hint to the UI for the amount of exposed parameters. A visibility of 2 means
   that the UI should hide the parameter for users, but show it for experts and
   advanced users.
   *remark: this 'inherits' from the module property. i.e. if it is not specified, the
   value of the module-property (if given) should be used instead*

-  **group** (optional) identifier, may contain ':' which may be interpreted as path separator.
   The ECS may group the parameters according to this property.
   The lowercase version of a group must not match any lowercase version of a parameter name
   of the same module.
   (*see:* `SECoP Issue 8: Groups and Hierarchy`_)

*remark: the parameter-property ``group`` is used for grouping of parameters within a module,
the module-property ``group`` is used for grouping of modules within a node.*

Data Types
==========
SECoP defines a very flexible data typing system. Data types are used to describe
the possible values of parameters and how they are serialised.
They may also impose restrictions on the useable values or amount of data.
Like the integer or fractional data types SECoP defines.
Also an Enum is defined for convenience of not having to remember the meaning of values from a reduced set.
A Bool datatype is similiar to a predefined Enum, but uses the JSON-values true and false.
(Of course 0 should be treated as False and 1 as True if a bool value isn't using these values.)

Furthermore, SECoP not only define basic data types but also structured datatypes.
Tuples allow to combine a fixed amount of values with different datatypes in an ordered way to be used as one.
Arrays store a given number of dataelements having the same datatype.
Structs are comparable to tuples, with the differenc of using named entries whose order is irrelevant during transport.

.. contents::
    :depth: 1
    :local:
    :backlinks: entry

double
------

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["double"] *or*
        | ["double", <min>] *or*
        | ["double", <min>, <max>]
        |
        | if <max> is not given or null, there is no upper limit
        | if <min> is null or not given, there is no lower limit

    * - Transport example
      - | as JSON-number:
        | 3.14159265

    * - Datatype in C/C++
      - | double

int
---

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["int"] *or*
        | ["int", <min>] *or*
        | ["int", <min>, <max>]
        |
        | if <max> is not given or null, there is no upper limit
        | if <min> is null or not given, there is no lower limit

    * - Transport example
      - | as JSON-number:
        | -55

    * - Datatype in C/C++
      - | int64_t

bool
----

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["bool"]

    * - Transport example
      - | as JSON-boolean: true or false
        | true

    * - Datatype in C/C++
      - | int64_t

enum
----

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["enum", {<name> : <value>, ....}]

    * - Transport example
      - | as JSON-number, the client performs the mapping back to the name:
        | 2

    * - Datatype in C/C++
      - | int64_t

string
------

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["string"] *or*
        | ["string", <max len>] *or*
        | ["string", <max len>, <min len>]
        |
        | if <max len> is not given, it is assumed as 255.
        | if <min len> is not given, it is assumed as 0.
        | if the string is UTF-8 encoded, the length is counting the number of bytes, not characters

    * - Transport example
      - | as JSON-string:
        | "hello!"

    * - Datatype in C/C++ API
      - | char \*

blob
----

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["blob", <max len>] *or*
        | ["blob", <max len>, <min len>]
        |
        | if <min len> is not given, it is assumed as 1.

    * - Transport example
      - | as base64 (see :RFC:`4648`) encoded JSON-string:
        | "AA=="

    * - Datatype in C/C++ API
      - | *(proposed)*
        | struct {int64_t len, char \*data}

array
-----

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["array", <basic type>, <max len>] *or*
        | ["array", <basic type>, <max len>, <min len>]
        |
        | if <min len> is not given, it is assumed as 0.
        | the length is the number of elements

    * - Transport example
      - | as JSON-array:
        | [3,4,7,2,1]

    * - Datatype in C/C++ API
      - | <basic_datatype>[]

tuple
-----

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["tuple", [<datatype>, <datatype>, ...]]

    * - Transport example
      - | as JSON-array:
        | [0,"idle"]

    * - Datatype in C/C++ API
      - | struct

struct
------

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["struct", {<name> : <datatype>, <name>: <datatype>, ....}]

    * - Transport example
      - | as JSON-object:
        | {"x": 0, "y": 1}

    * - Datatype in C/C++ API
      - | struct
        |
        | might be null

*remark: see also* `SECoP Issue 35: Partial structs`_

command
-------

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ["command"] *or*
        | ["command", <argumenttype>] *or*
        | ["command", <argumenttype>, <resulttype>]]
        |
        | if <argumenttype> is not given or null, the command has no argument
        | if <resulttype> is null or not given, the command returns no result
        | only one argument is allowed, though several arguments may be used if
        | encapsulated in a structural datatype (struct, tuple or array).
        | If such encapsulation or data grouping is needed, a struct SHOULD be used.
        | In any case, the meaning of result and argument(s) SHOULD be written down
        | in the description of the command.

    * - Message examples
      - | do module:stop null
        | done module:stop [null,{t:123456789.1}]
        |
        | do module:communicate "Hello"
        | done module:communicate ["World!",{t:123456789.2}]
        |
        | do module:uploadcurve {"curve":57, "sensor":"X1234A", "points":[[1, 3.4,....

*remark: see also* `SECoP Issue 35: Partial structs`_


Future Compatibility
====================
This specification defines a set of requests and replies above.
Only those messages are ALLOWED to be generated by any software complying to this specification:

.. compound::
    Requests:

    .. image:: images/defined_requests.png
       :alt: defined_requests

.. compound::
    Replies:

    .. image:: images/defined_replies.png
       :alt: defined_replies

The specification is intended to grow and adopt to new needs.
To future proof the the communication the following messages MUST be parsed and treated correctly
(i.e. the ignored_value part is to be ignored).

.. compound::
    Requests:

    .. image:: images/must_accept_requests.png
       :alt: must_accept_requests

.. compound::
    Replies:

    .. image:: images/must_accept_replies.png
       :alt: must_accept_replies

As a special case, an argumentless command may also by called without specifying the data part.
In this case an argument of null is to be assumed.
Also an argumentless ping is to be handled as a ping request with an empty token string.
The corresponding reply then contains a double space. This MUST also be parsed correctly.

Similiarly, the reports need to be handled like this:

.. compound::
    Data report:

    .. image:: images/data_report.png
       :alt: data_report ::= "[" json-value "," qualifiers ("," ignored_value)* "]"

.. compound::
    Error report:

    .. image:: images/error_report.png
       :alt: error_report ::= '["' copy_of_request '","' error_msg '",' error_info ("," ignored_value)* "]"

Complying to these rules maximise to possibility of future + backwards compatibility.


Licences
========

The above diagrams were generated using http://bottlecaps.de/rr/ui by Gunther Rademacher.
The author approved using these images here. The licence reads as follows::

    Railroad Diagram Generator is subject to

        Copyright 2010-2018 Gunther Rademacher <grd@gmx.net>
        All rights reserved.

    Portions of source code, that are exposed in generated files, are
    released under the Apache 2.0 License:

        Copyright 2010-2018 Gunther Rademacher <grd@gmx.net>

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

            http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
        implied. See the License for the specific language governing
        permissions and limitations under the License.

    Thank you for choosing Railroad Diagram Generator.



.. _`SECoP Issue 3:Timestamp Format`: issues/003c%20Timestamp%20Format.rst
.. _`SECoP Issue 4: The Timeout SEC Node Property`: issues/004c%20The%20Timeout%20SEC%20Node%20Property.rst
.. _`SECoP Issue 6: Keep Alive`: issues/006c%20Keep%20Alive.rst
.. _`SECoP Issue 7:Time Synchronization`: issues/007c%20Time%20Synchronization.rst
.. _`SECoP Issue 8: Groups and Hierarchy`: issues/008c%20Groups%20and%20Hierarchy.rst
.. _`SECoP Issue 9: Module Meaning` : issues/009c%20Module%20Meaning.rst
.. _`SECoP Issue 18: Interface classes`: issues/018d%20Interface%20Classes.rst
.. _`SECoP Issue 22: Enable Module instead of Shutdown Command`: issues/022u%20Enable%20Module%20instead%20of%20Shutdown%20Command.rst
.. _`SECoP Issue 26: More Module Meanings`: issues/026d%20More%20Module%20Meanings.rst
.. _`SECoP Issue 28: Clarify buffering mechanism`: issues/028p%20Clarify%20buffering%20mechanism.rst
.. _`SECoP Issue 29: New messages for buffering`: issues/029p%20New%20messages%20for%20buffering.rst
.. _`SECoP Issue 35: Partial structs`: issues/035p%20Partial%20structs.rst
