Overview
========

This section will give an overview over the different parts of SECoP described
in the different chapters of the specification.  As this specification is
written bottom-up, and not everyone might need the details from every chapter,
this part is intended to make the reader familiar with the basic concepts needed
for understanding each chapter on its own.

We will start with the structural concept for the SECoP server, move on to the
messages and in the end learn what the description is for a SECoP server and
which datatypes are supported.


SECoP Node Structure
--------------------

- SECoP is a communication protocol with a client-server model.  The server is
  called the "SEC node".

- Each SEC node consists of one or more :ref:`Modules`.  These represent the
  main interface for interacting with the hardware.

- Each of these modules can have static values which are known at startup and
  dynamic values.  These are called :ref:`properties` and :ref:`Parameters
  <accessibles>` respectively.  Parameters can in turn have their own
  Properties.

  Examples of properties would be the datatype of the parameter or the
  :ref:`readonly <prop-readonly>` flag that shows whether a parameter may be
  written to or not.

  To initiate actions that may not necessarily be tied to a parameter, modules
  can also have :ref:`Commands <accessibles>`, like stopping the current
  movement or running a calibration.

- To show which capabilities a module supports, and to give a name to common
  groupings, there are :ref:`interface-classes` and :ref:`features`.

  The four interface classes currently defined are :ref:`Communicator
  <Communicator>`, which is intended for bare request-response communication,
  and three that represent values that can be interacted with: :ref:`Readable
  <Readable>`, :ref:`Writable <Writable>` and :ref:`Drivable <Drivable>`.

  The only feature currently defined is :ref:`HasOffset <HasOffset>`.

- Readable modules have a ``value`` and a ``status`` parameters, which show the
  current error state of the module.  As the name implies, the main value can
  only be read.

- Writable and Drivable modules allow influencing their value by writing to a
  ``target`` parameter.  Writables are intended for fast operations like
  switches, where the target state can be reached quickly (i.e. sub-second
  operations or similar).  Drivables funcion similar, but they can take longer
  to reach the target state.  Think of a motor that has to drive to a position
  along a rail.  For this, they would reflect the ongoing operation in their
  status, setting it to ``BUSY`` and returning to ``IDLE`` once the operation is
  complete.

Messages
--------

The communication between client and server builds upon :ref:`messages
<messages>` which are transferred between SEC node and client.

As an introduction, we will look at the general message structure, and the
messages :ref:`*IDN? <message-identification>`, :ref:`describe
<message-describe>`, :ref:`read <message-read>`, and :ref:`change
<message-change>`.  For an overview of all available messages, including calling
commands and remote logging, etc., see :ref:`messages`.

There are three parts a message can have: ``action``, ``identifier`` and
``data``. Of these, depending on the action, ``identifier`` and ``data`` may not
be needed.

- The first part, ``action``, specifies the kind of message we want to send.
- The middle part, ``identifier``, points to the module/parameter/property or
  command we want to operate on.
- Finally, ``data`` is the data that may be needed for the specified action,
  like the new value when writing to a parameter, or the argument of a command.

The default mode for communicating between SEC node and client is a classic
request-response mode, where the client initiates an exchange.  However, if the
SEC node implementation supports it, the client may choose to move to the
asynchronous mode, where the SEC node will send updates asynchronously when they
occur. For more details on this, see the :ref:`activate <message-activate>`
message.

As an example, writing the ``target`` parameter of a module we will name ``Temp``::

    change Temp:target 11.5

This will change the target parameter to the value 11.5, if possible.

For an example of a message where one or more of the components is optional,
lets have a look at the :ref:`describe <message-describe>` message::

    describe

As you can see, this message only needs the ``action`` part.

This brings us to the topic of the self-description of SECoP.


Description
-----------

The description is a formalized structure containing all information about the
SEC node's modules and their properties, parameters and commands.  It is machine
readable, with all details about modules, parameters, datatypes and so on
included.  Additionally, the implementor of the SEC node has to include textual
descriptions for the important parts of the SEC node.  These short documentation
texts are not intended for machines, but for the human operators of the
equipment.  Functional dependencies that have to be machine readable are exposed
through the already mentioned :ref:`interface-classes` and :ref:`features`.

For the representation details, see the section :ref:`descriptive-data-format`.


Data types
----------

A variety of datatypes are covered in SECoP.  There are simple datatypes, for
example:

- :ref:`Integer <int>`
- :ref:`Scaled Integer <scaled>`
- :ref:`Floating Point <double>`
- :ref:`Boolean <bool>`
- :ref:`Enum <enum>`
- :ref:`String <string>`
- :ref:`Blob <blob>`

For more complicated values, there are three structured datatypes:

- :ref:`Array <array>`, an array of uniform values
- :ref:`Tuple <tuple>`, a fixed sequence of items that can be of different types
- :ref:`Struct <struct>`, a collection of named members, each of which has its
  own type

.. note:: There is as of this writing no ``None``/``null`` value or "optional"
          datatype that can be transported over SECoP.
