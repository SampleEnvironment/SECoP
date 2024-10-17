.. _data-types:

Data Info
=========

SECoP defines a very flexible data typing system.  Data info structures are used
to describe the possible values of parameters and how they are serialized.  They
may also impose restrictions on the usable values or amount of data.  The data
info structure consists of the name of the datatype augmented by data properties
to pinpoint the exact meaning of the data to be described.

SECoP defines some basic data types for numeric quantities, like double_,
scaled_ and int_.  An enum_ is defined for convenience of not having to remember
the meaning of values from a reduced set.  A bool_ datatype is similar to a
predefined Enum, but uses the JSON values true and false.  For non-numeric
types, a string_ and a blob_ are defined as well.

Furthermore, SECoP not only defines basic data types, but also structured
datatypes.  tuple_ allows aggregation of a fixed amount of values with different
datatypes in an ordered way to be used as one.  array_ stores a variable number
of data elements having the same datatype.  struct_ is comparable to tuples,
with the difference of using named entries whose order is irrelevant during
transport.

For data types that specify limits, they are always inclusive, i.e. the value is
allowed to be one of the limit values.  Also, both limits may be set to the same
value, in which case there is just one allowed value.

All data info structures are specified in the descriptive data in the following
generic form:

.. image:: images/datatype.svg
    :alt: datatype ::= '{' datatype-name ':' '{' ( datatype-property ( ',' datatype-property )* )? '}'

Here is an overview of all defined data types:

.. contents::
    :depth: 1
    :local:
    :backlinks: entry

Depending on the data type, there are different sets of data properties
available.


.. _double:

Floating Point Numbers: ``double``
----------------------------------

Datatype to be used for all physical quantities.

The ECS SHOULD internally use IEEE-754 double floating point values and MUST
support AT LEAST the full IEEE-754 single float value range and precision.
However, NaN, infinity and denormalized numbers do not need to be supported, as
JSON can't transport those 'values'.

If the relative resolution is not given or not better than 1.2e-7, single
precision floats may be used in the ECS.

Related issue: :issue:`042 Requirements of datatypes`


Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"min"``
    Lower limit. If ``min`` is omitted, there is no lower limit.

``"max"``
    Upper limit. If ``max`` is omitted, there is no upper limit.

``"unit"``
    String giving the unit of the parameter.

    SHOULD be given, if meaningful.  The quantity is unitless if unit is omitted
    or the empty string.  Preferably SI units (including prefix) SHOULD be used.

    Related issue: :issue:`043 Parameters and units`

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

    Related issue: :issue:`049 Precision of Floating Point Values`

``"fmtstr"``
    A C-style format  string as a hint on how to format numeric parameters for
    the user.  Default value: ``"%.6g"``.

    The string must obey the following syntax:

    .. image:: images/fmtstr.svg
        :alt: fmtstr ::= "%" "." [1-9]? [0-9] ( "e" | "f" | "g" )


Example
~~~~~~~

.. code:: json

    {"type": "double", "min": 0, "max": 100, "fmtstr": "%.3f"}


Transport
~~~~~~~~~

As a JSON number.

Example: ``3.14159265``


.. _scaled:

Scaled Integer: ``scaled``
--------------------------

Scaled integers are transported as integers, but the physical value is a
floating point value.  It is up to the client to perform the conversion when
reading/writing.  The main motivation for this datatype is for SEC nodes with
limited capabilities, where floating point calculation is a major effort.

Related issue: :issue:`044 Scaled integers`


Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

``"scale"``
    A (numeric) scale factor to be multiplied with the transported integer.

``"min"``, ``"max"``
    The limits of the transported integer, ``min <= max``.  The limits of the
    represented floating point value are ``min*scale`` and ``max*scale``.


Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"unit"``
    String giving the unit of the paramete, as for double_.

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


Example
~~~~~~~

.. code:: json

    {"type": "scaled", "scale": 0.1, "min": 0, "max": 2500}

i.e. a value between 0.0 and 250.0.


Transport
~~~~~~~~~

As an integer JSON number.

Example: ``1255`` meaning 125.5 in the above example.


.. _int:

Integer: ``int``
----------------

Datatype to be used for integer numbers.  For any physical quantity ``double``
or ``scaled`` **SHOULD** be used.  An integer SHOULD have no unit and it SHOULD
be representable with signed 24 bits, i.e. all integers SHOULD fit inside -2\
:sup:`24` ... 2\ :sup:`24`, as some JSON libraries might parse JSON numbers
with 32bit float too.


Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

``"min"``, ``"max"``
   Integer limits, ``<min>`` <= ``<max>``.


Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"unit"``
    A string giving the unit of the parameter, as for double_.


Example
~~~~~~~

.. code:: json

    {"type": "int", "min": 0, "max": 100}


Transport
~~~~~~~~~

As a JSON number.

Example: ``-55``


.. _bool:
.. _boolean:

Boolean: ``bool``
-----------------

Syntax
~~~~~~

.. code:: json

    {"type": "bool"}


Transport
~~~~~~~~~

As JSON ``true`` or ``false``.


.. _enum:

Enumerated Type: ``enum``
-------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~

``"members"``
    A JSON object giving all possible values: ``{<name>: <value>, ....}``

    ``name``\ s are strings, ``value``\ s are (preferably small) integers.  Both
    ``name``\ s and ``value``\ s MUST be unique within an enum.


Example
~~~~~~~

.. code:: json

    {"type": "enum", "members": {"IDLE": 100, "WARN": 200, "BUSY": 300, "ERROR": 400}}


Transport
~~~~~~~~~

As a JSON-number.  The client may perform a mapping back to the name.

Example: ``200``


.. _string:

String: ``string``
------------------

Optional data properties
~~~~~~~~~~~~~~~~~~~~~~~~

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


Example
~~~~~~~

.. code:: json

    {"type": "string", "maxchars": 80}


Transport
~~~~~~~~~

As a JSON string.

Example: ``"Hello\n\u2343World!"``


.. _blob:

Binary Large Object: ``blob``
-----------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~

``"maxbytes"``
    The maximum length, counting the number of bytes (**not** the size of the
    encoded string).


Optional Data Property
~~~~~~~~~~~~~~~~~~~~~~

``"minbytes"``
   The minimum length, default is 0.


Example
~~~~~~~

.. code:: json

    {"type": "blob", "min": 1, "max": 64}


Transport
~~~~~~~~~

As a single-line base-64 (see :RFC:`4648`) encoded JSON string.

Example: ``"AA=="`` (a single, zero valued byte)


.. _array:

Sequence of Uniformly Typed Items: ``array``
--------------------------------------------

Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

``"members"``
    A nested datainfo, giving the datatype of the elements.

``"maxlen"``
    The maximum length, counting the number of elements.


Optional Data Property
~~~~~~~~~~~~~~~~~~~~~~

``"minlen"``
    The minimum length, default is 0.


Example
~~~~~~~

.. code:: json

    {"type": "array", "min": 3, "max": 10, "members": {"type": "int", "min": 0, "max": 9}}


Transport
~~~~~~~~~

As a JSON array.

example: ``[3,4,7,2,1]``


.. _tuple:

Finite Sequence of Items with Individually Typed Items: ``tuple``
-----------------------------------------------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~

``"members"``
    A JSON array listing the datatype for each member.  This also gives the
    number of members.


Example
~~~~~~~

.. code:: json

    {"type": "tuple", "members": [{"type": "int", "min": 0, "max": 999}, {"type": "string", "maxchars": 80}]}


Transport
~~~~~~~~~

As a JSON array.

Example: ``[300,"accelerating"]``


.. _Struct:

Collection of Named Items: ``struct``
-------------------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~

``"members"``
    A JSON object containing the names and datatypes of the members.


Optional Data Property
~~~~~~~~~~~~~~~~~~~~~~

``"optional"``
    A JSON list giving the names of optional struct elements.

    In 'change' and 'do' commands, the ECS might omit these elements, all other
    elements must be given.  The effect of a 'change' action with omitted
    elements should be the same as if the current values of these elements would
    have been sent with it.  The effect of a 'do' action with omitted elements
    is defined by the implementation.

    In all other messages (i.e. in replies and updates), all elements have to be
    given.


Example
~~~~~~~

.. code:: json

    {"type": "struct", "members": {"y": {"type": "double"},
                                   "x": {"type": "enum", "members": {"On": 1, "Off": 0}}}}

Transport
~~~~~~~~~

As a JSON object.

Example: ``{"x": 0.5, "y": 1}``

Related issue: :issue:`035 Partial Structs`


.. _command:

Commands: ``command``
---------------------

If an accessible is a command, its main datatype is ``command``.
Argument and result data are described within.

Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"argument"``
    The datatype of the single argument, or ``null``.

    Only one argument is allowed, but it can be a structural datatype with
    multiple values (struct, tuple or array).  If such encapsulation or data
    grouping is needed, a struct SHOULD be used.

``"result"``
    The datatype of the single result, or ``null``.

The meaning of result and argument(s) SHOULD be written down in the description
of the command.


Example
~~~~~~~

.. code:: json

    {"type": "command", "argument": {"type": "bool"}, "result": {"type": "int"}}


Transport
~~~~~~~~~

Command values are not transported as such.  But commands may be called
(i.e. executed) by an ECS.  Example:

.. code::

    > do module:invert true
    < done module:invert [72,{t:123456789.2}]
