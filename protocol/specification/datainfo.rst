.. _`data-types`:

Data info
=========

SECoP defines a very flexible data typing system. Data info structures are used to describe
the possible values of parameters and how they are serialized.
They may also impose restrictions on the usable values or amount of data.
The data info structure consists of the name of the datatype augmented by data-properties to pinpoint the exact meaning of the data to be described.

SECoP defines some basic data types for numeric quantities, like double_ and integer_.
An enum_ is defined for convenience of not having to remember the meaning of values from a reduced set.
A bool_ datatype is similar to a predefined Enum, but uses the JSON-values true and false.
(Of course 0 should be treated as False and 1 as True if a bool value isn't using the JSON literals.)
For non-numeric types, a string_ and a blob_ are defined as well.

Furthermore, SECoP not only defines basic data types but also structured datatypes.
Tuples allow to combine a fixed amount of values with different datatypes in an ordered way to be used as one.
Arrays store a given number of data elements having the same datatype.
Structs are comparable to tuples, with the difference of using named entries whose order is irrelevant during transport.

The limits, which have to be specified with the data info, are always inclusive,
i.e. the value is allowed to have one of the values of the limits.
Also, both limits may be set to the same value, in which case there is just one allowed value.

All data info structures are specified in the descriptive data in the following generic form:

.. image:: images/datatype.svg
    :alt: datatype ::= '{' datatype-name ':' '{' ( datatype-property ( ',' datatype-property )* )? '}'


Here is an overview of all defined data types:

.. contents::
    :depth: 1
    :local:
    :backlinks: entry

Depending on the data type, there are different sets of data-properties available.

.. _double:

Floating Point Numbers: ``double``
----------------------------------

Datatype to be used for all physical quantities.

.. note::
    The ECS SHOULD use internally IEEE-754 double floating point values and MUST support AT LEAST
    the full IEEE-754 single float value range and precision. However, NaN, infinite and
    denormalized numbers do not need to be supported, as JSON can't transport those 'values'.

    If the relative resolution is not given or not better than 1.2e-7, single precision floats
    may be used in the ECS.

    :related issue: :issue:`042 Requirements of datatypes`

Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"min"``
    lower limit. if min is omitted, there is no lower limit

``"max"``
    upper limit. if max is omitted, there is no upper limit

``"unit"``
    string giving the unit of the parameter.

    SHOULD be given, if meaningful. Unitless if omitted or empty string.
    Preferably SI-units (including prefix) SHOULD be used.

    :related: :issue:`043 Parameters and units`

``"absolute_resolution"``
    JSON-number specifying the smallest difference between distinct values.
    default value: 0

``"relative_resolution"``
    JSON-number specifying the smallest relative difference between distinct values:

    ``abs(a-b) <= relative_resolution * max(abs(a),abs(b))``

    default value: 1.2e-7 (enough for single precision floats)

    if both ``absolute_resolution`` and ``relative_resolution`` are given, the expected
    resolution is:

    ``max(absolute_resolution, abs(value) * relative_resolution)``

    :related: :issue:`049 Precision of Floating Point Values`

``"fmtstr"``
    string as a hint on how to format numeric parameters for the user.
    default value: "%.6g"

    The string must obey the following syntax:

    .. image:: images/fmtstr.svg
        :alt: fmtstr ::= "%" "." [1-9]? [0-9] ( "e" | "f" | "g" )


Example
~~~~~~~

.. code:: json

   {"type": "double", "min": 0, "max": 100, "fmtstr": "%.3f"}

Transport
~~~~~~~~~
as JSON-number

example: ``3.14159265``

.. _scaled:

Scaled Integer: ``scaled``
--------------------------

Scaled integers are to be treated as 'double' in the ECS, they are just transported
differently. The main motivation for this datatype is for SEC nodes with limited
capabilities, where floating point calculation is a major effort.


Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

``"scale"``
    a (numeric) scale factor to be multiplied with the transported integer

``"min"``, ``"max"``
    The limits of the transported integer. ``<min>`` <= ``<max>``.
    The limits of the represented floating point value are ``<min>*<scale>, <max>*<scale>``

Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"unit"``
    string giving the unit of the parameter. (see datatype double_)

``"absolute_resolution"``
    JSON-number specifying the smallest difference between distinct values.

    default value: ``<scale>``

``"relative_resolution"``
    JSON-number specifying the smallest relative difference between distinct values:

    ``abs(a-b) <= relative_resolution * max(abs(a),abs(b))``

    default value: 1.2e-7 (enough for single precision floats)

    if both ``absolute_resolution`` and ``relative_resolution`` are given, the expected
    resolution is:

    ``max(absolute_resolution, abs(value) * relative_resolution)``

    :related: :issue:`049 Precision of Floating Point Values`

``"fmtstr"``
    string as a hint on how to format numeric parameters for the user.
    default value: "%.<n>f" where <n> = max(0,-floor(log10(scale)))

    The string must obey the same syntax as above for double_.

Example
~~~~~~~

.. code:: json

   {"type": "scaled", "scale": 0.1, "min": 0, "max": 2500}

i.e. a value between 0.0 and 250.0.

Transport
~~~~~~~~~
an integer JSON-number

for example ``1255`` meaning 125.5 in the above example.


:related issue: :issue:`044 Scaled integers`.

.. _int:
.. _integer:

Integer: ``int``
----------------

Datatype to be used for integer numbers.
For any physical quantity ``double`` or ``scaled`` **SHOULD** be used.
An integer SHOULD have no unit and it SHOULD be representable with signed 24 bits (i.e. all integers SHOULD fit
inside -2\ :sup:`24` ... 2\ :sup:`24`), as some JSON libraries might parse JSON-numbers
with 32bit float too.

Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~
``"min"``, ``"max"``
   integer limits, ``<min>`` <= ``<max>``

Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"unit"``
    string giving the unit of the parameter. (see datatype double_)

Example
~~~~~~~

.. code:: json

   {"type": "int", "min": 0, "max": 100}

Transport
~~~~~~~~~
as JSON-number

example: ``-55``

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
``true`` or ``false``


.. _enum:

Enumerated Type: ``enum``
-------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~
``"members"``
    a JSON-object: ``{<name> : <value>, ....}``

    ``name``\ s are strings, ``value``\ s are (small) integers, both ``name``\ s and ``value``\ s MUST be unique

Example
~~~~~~~
.. code:: json

   {"type": "enum", "members": {"IDLE": 100, "WARN": 200, "BUSY": 300, "ERROR": 400}}

Transport
~~~~~~~~~
as JSON-number, the client may perform a mapping back to the name

example: ``200``


.. _string:

String: ``string``
------------------

Optional data properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"maxchars"``
    the maximum length of the string in UTF-8 code points, counting the number of characters (**not** bytes!)

    :note:
        an UTF-8 encoded character may occupy up to 4 bytes.
        Also the end-of-string marker may need another byte for storage.

``"minchars"``
    the minimum length, default is 0

``"isUTF8"``
    boolean, if UTF8 character set is allowed for values, or if the value is allowed only
    to contain 7Bit ASCII characters (i.e. only code points < 128), each occupying a single byte.
    defaults to **False** if not given.

Example
~~~~~~~
.. code:: json

   {"type": "string", "maxchars": 80}

Transport
~~~~~~~~~
as JSON-string

example: ``"Hello\n\u2343World!"``

.. _Blob:

Binary Large Object: ``blob``
-----------------------------

Mandatory Data Property
~~~~~~~~~~~~~~~~~~~~~~~
``"maxbytes"``
    the maximum length, counting the number of bytes (**not** the size of the encoded string)

Optional Data Property
~~~~~~~~~~~~~~~~~~~~~~
``"minbytes"``
   the minimum length, default is 0

Example
~~~~~~~
.. code:: json

   {"type": "blob", "min": 1, "max": 64}

Transport
~~~~~~~~~
as single-line base64 (see :RFC:`4648`) encoded JSON-string

example: ``"AA=="`` (a single, zero valued byte)

.. _array:

Sequence of Equally Typed Items : ``array``
-------------------------------------------

Mandatory Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~~

``"members"``
    the datatype of the elements

``"maxlen"``
    the maximum length, counting the number of elements

Optional Data Property
~~~~~~~~~~~~~~~~~~~~~~

``"minlen"``
    the minimum length, default is 0

Example
~~~~~~~
.. code:: json

   {"type": "array", "min": 3, "max": 10, "members": {"type": "int", "min": 0, "max": 9}}

Transport
~~~~~~~~~
as JSON-array

example: ``[3,4,7,2,1]``

.. _tuple:

Finite Sequence of Items with Individually Defined Type: ``tuple``
------------------------------------------------------------------

Mandatory data property
~~~~~~~~~~~~~~~~~~~~~~~
``"members"``
    a JSON array listing the datatypes of the members

Example
~~~~~~~
.. code:: json

   {"type": "tuple", "members": [{"type": "int", "min": 0, "max": 999}, {"type": "string", "maxchars": 80}]}

Transport
~~~~~~~~~
as JSON-array

.. code:: json

   [300,"accelerating"]


.. _Struct:

Collection of Named Items: ``struct``
-------------------------------------

Mandatory data property
~~~~~~~~~~~~~~~~~~~~~~~
``"members"``
    a JSON object containing the names and datatypes of the members

Optional data property
~~~~~~~~~~~~~~~~~~~~~~
``"optional"``
    the names of optional struct elements is given

    In 'change' and 'do' commands, the ECS might omit these elements, all other
    elements must be given.
    The effect of a 'change' action with omitted elements should be the same
    as if the current values of these elements would have been sent with it.
    The effect of a 'do' action with omitted elements is defined by the implementation.

    In all other messages (i.e. in replies and updates), all elements have to be given.

Example
~~~~~~~
.. code:: json

   {"type": "struct", "members": {"y": {"type": "double"},
                                  "x": {"type": "enum", "members": {"On": 1, "Off": 0}}}}

Transport
~~~~~~~~~
as JSON-object

example: ``{"x": 0.5, "y": 1}``

:related issue: :issue:`035 Partial Structs`


.. _command:

Command-flag for Accessibles
----------------------------

If an accessible is a command, its argument and result is described by the ``command`` datatype.

Optional Data Properties
~~~~~~~~~~~~~~~~~~~~~~~~

``"argument"``
    the datatype of the single argument, or ``null``.

    only one argument is allowed, though several arguments may be used if
    encapsulated in a structural datatype (struct, tuple or array).
    If such encapsulation or data grouping is needed, a struct SHOULD be used.

``"result"``
    the datatype of the single result, or ``null``.

    In any case, the meaning of result and argument(s) SHOULD be written down
    in the description of the command.

Example
~~~~~~~
.. code:: json

   {"type": "command", "argument": {"type": "bool"}, "result": {"type": "bool"}}


Transport Example
~~~~~~~~~~~~~~~~~
Command values are not transported as such. But commands may be called (i.e. executed) by an ECS.
Example:

.. code::

    > do module:invert true
    < done module:invert [false,{t:123456789.2}]


