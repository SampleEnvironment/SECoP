SECoP Issue 55: Reformat Datatype description (closed)
======================================================

Motivation
----------

The `SECoP issue 52`_ introduced a new data type handling, which is hardly
readable. Because these new properties are needed for SECoP and these
properties require a change of the base structure (and a rewrite of base
functionality at least at the HZB approach), they should be formated in a
more convinient way. Some properties should not be mixed at different
locations and a JSON object fits better than a JSON array.

Proposal
--------

Niklas, Frank and Lutz had the idea to use json objects to this.
Introduce a key/value access for data type elements and properties with
a JSON object for data type description inside the "describing" message.

Instead of using an array for a data type (example for "double")

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ``["double", <min>, <max>]``
        | ``["double", <min>, <max>, {"unit": <unit>, "fmtstr": <fmtstr>, ...}]``

the data descriptor uses an object (example for "double") and a different
property name to reflect the additional information, which is more than a
data type only:

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "double",``
        |  ``"minimum_value": <min>,``
        |  ``"maximum_value": <max>,``
        |  ``"unit": <unit>,``
        |  ``"fmtstr": <fmtstr>,``
        |  ``... }``

Some names for data properties used in this proposal came from previous
proposals:

- `SECoP issue 42`_ (Requirements of datatypes)
- `SECoP issue 49`_ (Precision of Floating Point Values)
- `SECoP issue 50`_ (Reserved Names)

New data properties are "type", "members", "argument" and "result". These
data properties are reserved for their purpose, the transport format is
unchanged. The data properties themselves may be of simple json data types
only (boolean, numbers, string, null), no arrays nor objects are allowed
here to reduce complexity for a generic ECS handling. For the same reason,
custom data properties are not allowed. New data properties may be introduced
with a later discussed issues or extensions of the SECoP protocol.

There may be side effects with existing properties for a parameter or command
and data properties inside the data descriptor (e.g. "unit"). They have to be
discussed outside this proposal.

For completeness here is the definition of known SECoP data descriptors:

"bool" type
~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "bool",``
        |  ``... }``

    * - Example
      - | ``{ "type": "bool" }``

    * - Transport example
      - | as JSON-boolean: true or false
        | ``true``

"double" type
~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "double",``
        |  ``"minimum_value": <minimum_value>,``
        |  ``"maximum_value": <maximum_value>,``
        |  ``"unit": <unit>,``
        |  ``"fmtstr": <fmtstr>,``
        |  ``"precision": <precision>,``
        |  ``... }``
        |
        | ``minimum_value`` and ``maximum_value`` are required. They
        | may be ``null`` to show, there is none. If non-``null`` then
        | ``minimum_value`` and ``maximum_value`` are numbers with
        | ``minimum_value`` <= ``maximum_value``.

    * - Example
      - | ``{ "type": "double", "minimum_value": 0, "maximum_value": 100,``
        |  ``"unit": "mm", "fmtstr": "%.3f", "precision": 0.001 }``

    * - Transport example
      - | as JSON-number:
        | ``3.14159265``

"int" type
~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "int",``
        |  ``"minimum_value": <minimum_value>,``
        |  ``"maximum_value": <maximum_value>,``
        |  ``"unit": <unit>,``
        |  ``... }``
        |
        | ``minimum_value`` and ``maximum_value`` are required. They
        | may be ``null`` to show, there is none. If non-``null`` then
        | ``minimum_value`` and ``maximum_value`` are numbers with
        | ``minimum_value`` <= ``maximum_value``.

    * - Example
      - | ``{ "type": "int", "minimum_value": -100,``
        |  ``"maximum_value": 100, "unit": "steps" }``

    * - Transport example
      - | as JSON-number:
        | ``-55``

"enum" type
~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "enum",``
        |  ``"members": { <name>: <value>, ... },``
        |  ``... }``

    * - Example
      - ``{ "type": "enum", "members": {"IDLE":100,"WARN":200,"BUSY":300,"ERROR":400}}``

    * - Transport example
      - | as JSON-number, the client performs the mapping back to the name:
        | ``200``

"scaled" type
~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "scaled",``
        |  ``"scale": <scale_factor>,``
        |  ``"minimum_value": <minimum_value>,``
        |  ``"maximum_value": <maximum_value>,``
        |  ``"unit": <unit>,``
        |  ``"fmtstr": <fmtstr>,``
        |  ``... }``
        |
        | ``scale``, ``minimum_value`` and ``maximum_value`` are required.
        | ``minimum_value`` and ``maximum_value`` may be ``null`` to show,
        | there is none. If non-``null`` then ``minimum_value`` and
        | ``maximum_value`` are numbers with ``minimum_value`` <=
        | ``maximum_value``. ``scale`` has to be a number > 0.
        |
        | Remark:
        |   ``fmtstr`` might be guessed from ``scale``, but still an
        |   implementor may want to give it explicitly.

    * - Example
      - | ``{ "type": "scaled", "minimum_value": 0, "maximum_value": 250, "scale": 0.1 }``
        | i.e. a double value between 0.0 and 250.0
 
    * - Transport examples
      - | An integer JSON-number, ``1255`` meaning 125.5

"string" type
~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "string",``
        |  ``"minimum_bytes": <minimum_bytes>,``
        |  ``"maximum_bytes": <maximum_bytes>,``
        |  ``... }``
        |
        | ``minimum_bytes`` and ``maximum_bytes`` are integers with
        | ``minimum_bytes`` <= ``maximum_bytes``. The length is counting the
        | number of bytes (**not** characters!) used when the string is utf8
        | encoded!

    * - Example
      - ``{"type": "string", "minimum_bytes": 0, "maximum_bytes": 80}``

    * - Transport example
      - | as JSON-string:
        | ``"Hello\n\u2343World!"``

"blob" type
~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "blob",``
        |  ``"minimum_bytes": <minimum_bytes>,``
        |  ``"maximum_bytes": <maximum_bytes>,``
        |  ``... }``
        |
        | ``minimum_bytes`` and ``maximum_bytes`` are integers with
        | ``minimum_bytes`` <= ``maximum_bytes``. The length is counting the
        | number of bytes (**not** the size of the transport representation).

    * - Example
      - ``{"type": "blob", "minimum_bytes": 1, "maximum_bytes": 64}``

    * - Transport example
      - | as single-line base64 (see :RFC:`4648`) encoded JSON-string:
        | ``"AA=="``

"array" type
~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "array",``
        |  ``"members": { "type": <array-data-type>, ... }``
        |  ``"minimum_length": <minimum_length>,``
        |  ``"maximum_length": <maximum_length>,``
        |  ``... }``
        |
        | ``minimum_bytes`` and ``maximum_bytes`` are integers with
        | ``minimum_bytes`` <= ``maximum_bytes``. The length is the number
        | of elements.

    * - Example
      - | ``{"type": "array", "minimum_length": 3, "maximum_length": 10,``
        |  ``"members": {"type": "int", "minimum_value": 0, "maximum_value": 9}}``

    * - Transport example
      - | as JSON-array:
        | ``[3,4,7,2,1]``

"tuple" type
~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "tuple",``
        |  ``"members": [ { "type": <first-tuple-item-type>, ... }, ...]``
        |  ``... }``

    * - Example
      - | ``{"type": "tuple", "members": [``
        |  ``{"type": "int", "minimum_value": 0, "maximum_value": 999},``
        |  ``{"type": "string", "minimum_length": 0, "maximum_length": 99}``
        | ``]}``

    * - Transport example
      - | as JSON-array:
        | ``[300,"accelerating"]``

"struct" type
~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "struct",``
        |  ``"members": { <key> : { "type": <struct-item-type>, ... }, ...}``
        |  ``... }``

    * - Example
      - | ``{ "type": "struct", {``
        | ``"y": {"type": "int", "minimum_value": null, "maximum_value": null},``
        | ``"x": {"type": "enum", {"On":1, "Off":0}}}}``

    * - Transport example
      - | as JSON-object:
        | ``{"x": 0, "y": 1}``

"command" type
~~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datadescriptor
      - | ``{ "type": "command",``
        |  ``"argument": <argument-type>``
        |  ``"result": <result-type>``
        |  ``... }``
        |
        | If ``<argumenttype>`` is ``null``, the command has no argument.
        | If ``<resulttype>`` is ``null``, the command returns no result.
        | ``<argumenttype>`` and ``<resulttype>`` may be of any of the above
        | types. Only one argument and result is allowed, through several
        | arguments or results may be used if encapsulated in a structural
        | datatype (struct, tuple or array). If such encapsulation or data
        | grouping is needed, a struct SHOULD be used. In any case, the
        | meaning of result and argument SHOULD be written down in the
        | description of the command.

    * - Example
      - | ``{ "type": "command",``
        |  ``"argument": {"type": "bool"},``
        |  ``"result": {"type": "bool"}}``

    * - Transport examples
      - | > do module:invert true
        | < done module:invert [false,{t:123456789.2}]

.. _`SECoP issue 42`: 042%20Requirements%20of%20datatypes.rst
.. _`SECoP issue 49`: 049%20Precision%20of%20Floating%20Point%20Values.rst
.. _`SECoP issue 50`: 050%20Reserved%20Names.rst
.. _`SECoP issue 52`: 052%20Include%20Some%20Properties%20into%20Datatype.rst

Discussion
----------

Arguments against:

- less changes (there is already some code around which has to be changed)
- it less visible, that some things have to mandatory
- the message type is not necessarily at the beginning

Arguments for the change:

- it seems more logic, not to have two different places for "datatype properties":
  in a JSON array, and in a JSON object
- having everything in an JSON object is more explicit, because the names are given.


Decision on the Meeting 2019-03-21
----------------------------------

The datatype is a JSON-Array with two elements, the first is the message type,
the second a JSON object containing the datatype properties.
The names of the datatype properties follow the proposal above, except than
"min" and "max" is used instead of "mininum_..." and "maximum_..." for all
concerned types.

This way, the message type is clearly visible at the beginning, and the value
of the type is meaningful enough in order not to need a name.

Decision on the Meeting 2019-09-17
----------------------------------

The 'datatype' property is renamed to 'datainfo'.
'datainfo' is a JSON-object, where the 'type' element contains the basic data type,
the remaining elements contain the data properties (renamed from datatype properties).

Some data properties are changed/renamed:

``'string'``:
   * ``minchars``/``maxchars`` instead of ``min``/``max``, counting the number of
     character points instead of bytes in case of UTF-8 strings.
   * ``maxlen``is now optional
   * a new data property ``isUTF8`` (default: ``false``, which means it must be 7-bit ASCII coded, not containing NUL)
   
``'blob'``:
   ''minbytes``/``maxbytes`` instead of ``min``/``max``
   
``'array'``:
   ''minlen``/``maxlen`` instead of ``min``/``max``


The names of the datatype properties follow the proposal above, except than
"min" and "max" is used instead of "mininum_..." and "maximum_..." for all
concerned types.

This way, the message type is clearly visible at the beginning, and the value
of the type is meaningful enough in order not to need a name.

