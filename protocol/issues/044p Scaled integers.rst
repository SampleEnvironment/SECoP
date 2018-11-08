SECoP Issue 44: Scaled integers
===============================

Motivation
----------

One of the design decisions was to optimize the protocol to support low performance SEC-nodes
like PLC's or microcontrollers.
They may be overwhelmed if they need to support floating point datatypes, if that is not required for their function.
Typically on such systems you have either a fixed-point format or use integers which represent a fraction of one unit.

Proposal
--------

Include the definition of a 'scaled integer' datatype.
This datatype is to be transported as integer and both limits MUST be valid integers.
This datatype also needs to specify the 'scale', which is used to convert from/to real numbers.
I.e. the value it represents is the product of the scale and the integer value.
It may be used to represent physical quantities and is supposed to only be used on low-end SEC-nodes.
However, any ECS MUST support such SEC-nodes and hence this datatype as well.

scaled integer
~~~~~~~~~~~~~~

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ``["scaled", min, max, scale]``

    * - Example
      - ``["scaled", 0, 1000, 0.1]``

    * - Transport example
      - | as JSON-number without subdigits (i.e. as integer)
        | ``999``    (meaning 999*0.1 = 99.9 as real value)


Discussion
----------
not discussed in its present form.
