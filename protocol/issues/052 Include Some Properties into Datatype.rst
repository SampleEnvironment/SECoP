SECoP Issue 52: Include Some Properties into Datatype (proposed)
================================================================

Motivation
----------

The properties 'unit', 'fmtstr', 'absolute_precision' and 'relative_precision' should be moved
to the datatype declaration for the following reasons:

1) Programmatically, the functionality related to these properties are better stored
   together with the datatype.

2) It makes it possible to use these properties in doubles nested within structs or tuples

Proposal
--------

The datatypes double and scaled are extended to:

double
------

.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ``["double", <min>, <max>]``
        | ``["double", <min>, <max>, {"unit": <unit>, "fmtstr": <fmtstr>, ...}]``
        |
        | if ``<min>`` is ``null``, there is no upper limit
        | if ``<max>`` is ``null``, there is no lower limit
        | ``<min>`` and ``<max>`` are numbers with ``<min>`` <= ``<max>``
        | the fourth element is a json object, with the following possible members:
        | unit, fmtstr, absolute_precision, relative_precision

    * - Example
      - ``["double", 0, 100]``

    * - Transport example
      - | as JSON-number:
        | ``3.14159265``

scaled integers
---------------

Scaled integers are to be treated as 'double' in the ECS, they are just transported
differently. The main motivation for this datatype is for SEC nodes with limited
capabilities, where floating point calculation is a major effort.
For parameters with this type, it is not needed to indicate the properties 
'absolute_resolution' and 'fmtstr', as they can be derived from the datatype.


.. list-table::
    :widths: 20 80
    :stub-columns: 1

    * - Datatype
      - | ``["scaled", <min>, <max>, <scale>]``
        | ``["scaled", <min>, <max>, <scale>, {"unit": <unit>, ...}]``
        |
        | ``<min>`` and ``<max>`` MUST be given
        | ``<min>`` and ``<max>`` are integers with ``<min>`` <= ``<max>``
        | ``<scale>`` is a number
        | the fourth element is a json object, with the following possible members:
        | unit, fmtstr
        |
        | Remark:
        |   fmtstr might be guessed from scale, but still an implementor may want to
        |   give it explicitly. relative_precision equal to scale.

    * - Example
      - ``["scaled", 0, 250, 0.1]``
        i.e. a double value between 0.0 and 250.0
 
    * - Transport examples
      - | An integer JSON-number, ``1255`` meaning 125.5

The related items are removed from the list of predefined properties, and the
description of their meaning is moved to the Datatypes chapter.


Discussion
----------

