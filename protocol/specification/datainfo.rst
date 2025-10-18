.. _data-types:

Data types
==========

SECoP defines a very flexible data typing system.  The "Data info" structures
specified here are used to describe the possible values of parameters and how
they are serialized.  They may also impose restrictions on the usable values or
amount of data.

The data types are specified as JSON in the `datainfo` property of parameters
and commands.  An example for a floating-point valued parameter that can be in
the range (0,100) is:

.. code:: json

    {"type": "double", "min": 0, "max": 100, "fmtstr": "%.3f"}

SECoP defines some basic data types for numeric quantities, like double_,
scaled_ and int_.  An enum_ is defined for convenience of not having to remember
the meaning of values from a reduced set.  A bool_ datatype is similar to a
predefined enum, but uses the JSON values ``true`` and ``false``.  For
non-numeric types, a string_ and a blob_ are defined as well.  Finally, matrix_
can transport larger multi-dimensional homogeneous arrays.

Furthermore, SECoP not only defines basic data types, but also structured
datatypes.  tuple_ allows aggregation of a fixed amount of values with different
datatypes in an ordered way to be used as one.  array_ stores a variable number
of data elements having the same datatype.  struct_ is comparable to tuples,
with the difference of using named entries whose order is irrelevant during
transport.

For data types that specify limits, they are always inclusive, i.e. the value is
allowed to be one of the limit values.  Also, both limits may be set to the same
value, in which case there is just one allowed value.

Depending on the data type, there are different sets of data properties
available.

.. note:: There is as of this writing no ``None``/``null`` value or "optional"
          datatype that can be transported over SECoP.


.. _double:

``double``: Floating point number
---------------------------------

Datatype to be used for all physical quantities.

The ECS SHOULD internally use IEEE-754 double floating point values and MUST
support AT LEAST the full IEEE-754 single float value range and precision.
However, NaN, infinity and denormalized numbers do not need to be supported, as
JSON can't transport those 'values'.

If the relative resolution is not given or not better than 1.2e-7, single
precision floats may be used in the ECS.

.. dropdown:: Related issues

    | :issue:`042 Requirements of datatypes`

.. rubric:: Optional data properties

``"min"``
    Lower limit. If ``min`` is omitted, there is no lower limit.

``"max"``
    Upper limit. If ``max`` is omitted, there is no upper limit.

.. note::

    When a SEC node receives a `change` or `do` message with a value outside the
    allowed range ``[min, max]``, it MUST reply with an error message.  For
    readonly parameters, ``[min, max]`` indicate a trusted range.  A SEC node
    might send `update` or `reply` messages with values outside the trusted
    range, for example when the value is an extrapolation of the calibrated
    range.  The idea behind this relaxed rule is, that it is better for a SEC
    node to send an acquired value outside the range as it is - rather than
    change its value just to comply with the specified range.  The decision on
    how to treat such values is left to the ECS.

``"unit"``
    String giving the unit of the parameter.

    SHOULD be given, if meaningful.  The quantity is unitless if unit is omitted
    or the empty string.  Preferably SI units (including prefix) SHOULD be used.

    .. dropdown:: Related issues

        | :issue:`043 Parameters and units`

``"absolute_resolution"``
    A JSON number specifying the smallest difference between distinct values.
    Default value: 0.

``"relative_resolution"``
    A JSON number specifying the smallest relative difference between distinct
    values:

    ``abs(a - b) <= relative_resolution * max(abs(a), abs(b))``

    Default value: 1.2e-7 (enough for single precision floats).

    If both ``absolute_resolution`` and ``relative_resolution`` are given, the
    expected resolution is:

    ``max(absolute_resolution, abs(value) * relative_resolution)``

    .. dropdown:: Related issues

        | :issue:`049 Precision of Floating Point Values`

``"fmtstr"``
    A C-style format  string as a hint on how to format numeric parameters for
    the user.  Default value: ``"%.6g"``.

    The string must obey the following syntax:

    .. image:: images/fmtstr.svg
        :alt: fmtstr ::= "%" "." [1-9]? [0-9] ( "e" | "f" | "g" )

.. rubric:: Example

.. code:: json

    {"type": "double", "min": 0, "max": 100, "fmtstr": "%.3f"}

.. rubric:: Transport

As a JSON number.

Example: ``3.14159265``


.. _scaled:

``scaled``: Scaled integer
--------------------------

Scaled integers are transported as integers, but the physical value is a
floating point value.  It is up to the client to perform the conversion when
reading/writing.  The main motivation for this datatype is for SEC nodes with
limited capabilities, where floating point calculation is a major effort.

.. dropdown:: Related issues

    | :issue:`044 Scaled integers`

.. rubric:: Mandatory data properties

``"scale"``
    A (numeric) scale factor to be multiplied with the transported integer.

``"min"``, ``"max"``
    The limits of the transported integer, ``min <= max``.  The limits of the
    represented floating point value are ``min*scale`` and ``max*scale``.
    See also the note on the ``"min"`` and ``"max"`` properties of the
    double_ datatype.

.. rubric:: Optional data properties

``"unit"``
    String giving the unit of the parameter, as for double_.

``"absolute_resolution"``
    A JSON number specifying the smallest difference between distinct values.

    Default value: ``<scale>``

``"relative_resolution"``
    A JSON number specifying the smallest relative difference between distinct
    values, as for double_.

``"fmtstr"``
    A string as a hint on how to format values (after conversion) for the user.
    Default value: ``"%.<n>f"`` where ``<n> = max(0, -floor(log10(scale)))``.

    The string must obey the same syntax as above for double_.

.. rubric:: Example

.. code:: json

    {"type": "scaled", "scale": 0.1, "min": 0, "max": 2500}

i.e. a value between 0.0 and 250.0.

.. rubric:: Transport

As an integer JSON number.

Example: ``1255`` meaning 125.5 in the above example.


.. _int:

``int``: Integer
----------------

Datatype to be used for integer numbers.  For any physical quantity double_ or
scaled_ SHOULD be used.  An integer SHOULD have no unit and it SHOULD be
representable with signed 24 bits, i.e. all integers SHOULD fit inside -2\
:sup:`24` ... 2\ :sup:`24`, as some JSON libraries might parse JSON numbers with
32bit float too.

.. rubric:: Mandatory data properties

``"min"``, ``"max"``
    Integer limits, ``min <= max``.
    See also the note on the ``"min"`` and ``"max"`` properties of the
    double_ datatype.

.. rubric:: Optional data properties

``"unit"``
    A string giving the unit of the parameter, as for double_.

.. rubric:: Example

.. code:: json

    {"type": "int", "min": 0, "max": 100}

.. rubric:: Transport

As a JSON number.

Example: ``-55``


.. _bool:
.. _boolean:

``bool``: Boolean
-----------------

.. rubric:: Syntax

.. code:: json

    {"type": "bool"}

.. rubric:: Transport

As JSON ``true`` or ``false``.


.. _enum:

``enum``: Enumerated type
-------------------------

Datatype to be used for values that can only have a set of predefined values.

.. rubric:: Mandatory data property

``"members"``
    A JSON object giving all possible values: ``{<name>: <value>, ...}``

    ``name``\ s are strings, ``value``\ s are (preferably small) integers.  Both
    ``name``\ s and ``value``\ s MUST be unique within an enum.

.. rubric:: Example

.. code:: json

    {"type": "enum", "members": {"IDLE": 100, "WARN": 200, "BUSY": 300, "ERROR": 400}}

.. rubric:: Transport

As a JSON number.  The client may perform a mapping back to the name.

Example: ``200``


.. _string:

``string``: String
------------------

For human-readable strings.  Use blob_ for binary data.

.. rubric:: Optional data properties

``"maxchars"``
    The maximum length of the string in UTF-8 code points, counting the number
    of characters (**not** bytes).

``"minchars"``
    The minimum length, default is 0.

``"isUTF8"``
    Boolean specifying if the UTF-8 character set is allowed for values, or if
    the value is allowed only to contain 7-bit ASCII characters (i.e. only code
    points < 128), each occupying a single byte.

    Defaults to **False** if not given.

.. rubric:: Example

.. code:: json

    {"type": "string", "maxchars": 80}

.. rubric:: Transport

As a JSON string.

Example: ``"Hello\n\u2343World!"``


.. _blob:

``blob``: Binary large object
-----------------------------

.. rubric:: Mandatory data property

``"maxbytes"``
    The maximum length, counting the number of bytes (**not** the size of the
    encoded string).

.. rubric:: Optional data property

``"minbytes"``
    The minimum length, default is 0.

.. rubric:: Example

.. code:: json

    {"type": "blob", "min": 1, "max": 64}

.. rubric:: Transport

As a single-line base-64 (see :rfc:`4648`) encoded JSON string.

Examples:

| ``"AA=="`` (a single, zero valued byte)
| ``"U0VDb1A="`` (the ASCII string "SECoP")


.. _array:

``array``: Sequence of uniformly typed items
--------------------------------------------

.. rubric:: Mandatory data properties

``"members"``
    A nested datainfo, giving the datatype of the elements.

``"maxlen"``
    The maximum length, counting the number of elements.

.. rubric:: Optional data property

``"minlen"``
    The minimum length, default is 0.

.. rubric:: Example

.. code:: json

    {"type": "array", "min": 3, "max": 10, "members": {"type": "int", "min": 0, "max": 9}}

.. rubric:: Transport

As a JSON array.

Example: ``[3,4,7,2,1]``


.. _tuple:

``tuple``: Fixed sequence of individually typed items
-----------------------------------------------------

.. rubric:: Mandatory data property

``"members"``
    A JSON array listing the datatype for each member.  This also gives the
    number of members.

.. rubric:: Example

.. code:: json

    {"type": "tuple", "members": [{"type": "int", "min": 0, "max": 999}, {"type": "string", "maxchars": 80}]}

.. rubric:: Transport

As a JSON array.

Example: ``[300,"accelerating"]``


.. _struct:

``struct``: Collection of named items
-------------------------------------

This data type allows you to combine multiple named data members in a single
value.

.. rubric:: Mandatory data property

``"members"``
    A JSON object containing the names and datatypes of the members.

.. rubric:: Optional data property

``"optional"``
    A JSON list giving the names of optional struct elements.

    In `change` and `do` commands, the ECS might omit these elements, all other
    elements must be given.  The effect of a `change` action with omitted
    elements should be the same as if the current values of these elements would
    have been sent with it.  The effect of a `do` action with omitted elements
    is defined by the implementation.

    In all other messages (i.e. in replies and updates), all elements have to be
    given.

.. rubric:: Example

.. code:: json

    {"type": "struct", "members": {"y": {"type": "double"},
                                   "x": {"type": "enum", "members": {"On": 1, "Off": 0}}}}

.. rubric:: Transport

As a JSON object.

Example: ``{"x": 0.5, "y": 1}``

.. dropdown:: Related issues

    | :issue:`035 Partial Structs`


.. _matrix:

``matrix``: Binary multi-dimensional matrix
-------------------------------------------

Type for transferring a medium to large amount of homogeneous arrays with
potentially multiple dimensions.

At the moment, the type intends direct transfer of the data within the JSON
data.  It could be extended later to allow referring to a side-channel for
obtaining the data.

.. rubric:: Mandatory data properties

``"names"``
    A list of names for each dimension in the data.

``"maxlen"``
    A list of maximum lengths for each dimension. The actual lengths can vary
    but may not exceed these limits.

``"elementtype"``
    A string defining the type of each element, as a combination of three parts:

    - ``<`` or ``>`` to indicate little or big endianness.
    - ``i``, ``u``, ``f`` to indicate signed or unsigned integers or floating
      point numbers.
    - a number to indicate the number of bytes per element (1, 2, 4 or 8).

    Example: ``"<u4"`` is a little-endian encoded 32-bit unsigned integer.

.. rubric:: Optional data property

``"compression"``
    A string defining if and how the data is each ``blob`` is compressed.
    Currently, no compression types are defined.

.. rubric:: Example

.. code:: json

    {"type": "matrix", "elementtype": "<f4", "names": ["x", "y"], "maxlen": [100, 100]}

.. rubric:: Transport

As a JSON object containing the following items:

``"len"``
    List of the actual length of each dimension in the data.

``"blob"``
    The data, encoded as a single-line base64 (see :rfc:`4648`) encoded
    JSON string.

Example: ``{"len": [2, 3], "blob": "AACAPwAAAEAAAEBAAACAQAAAoEAAAMBA"}``

The order of the matrix elements is defined so that the first dimension
named in ``names`` (and listed in ``maxlen``/``len``) varies the fastest.

In this example, the result of decoding ``blob`` as a flat sequence of 4-byte
floats is ``[1, 2, 3, 4, 5, 6]``.  Then the matrix looks as follows::

  .     x=0 x=1
  y=0   1   2
  y=1   3   4
  y=2   5   6


.. _command:

``command``: Command
--------------------

If an accessible is a command, its main datatype is ``command``.  Argument and
result data are described within.

.. rubric:: Optional data properties

``"argument"``
    The datatype of the single argument, or ``null``.

    Only one argument is allowed, but it can be a structural datatype with
    multiple values (struct, tuple or array).  If such encapsulation or data
    grouping is needed, a struct SHOULD be used.

``"result"``
    The datatype of the single result, or ``null``.

The meaning of result and argument(s) SHOULD be written down in the description
of the command.

.. rubric:: Examples

.. code:: json

    {"type": "command", "argument": {"type": "bool"}, "result": {"type": "int"}}

    {"type": "command",
     "argument": {"type": "struct", "members": {"p": {"type": "double"},
                                                "i": {"type": "double"},
                                                "d": {"type": "double"}}},
     "result": {"type": "tuple", "members": [{"type": "int"}, {"type": "string"}]}}

.. rubric:: Transport

Command values are not transported as such.  But commands may be called
(i.e. executed) by an ECS.  Example calling the command with the type of the
second example above::

    > do module:setpid {"p": 100.0, "i": 5.0, "d": 1.2}
    < done module:setpid [[42, "control active"], {"t": 123456789.2}]
