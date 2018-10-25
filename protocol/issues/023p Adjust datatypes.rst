SECoP Issue 23: Adjust Datatypes
================================

Motivation
----------
The current definition of the ``tuple`` datatape uses superflous nesting.
The current wording of the ``struct`` datatype is difficult.

Proposal
--------
1. change the datatype of ``tuple``
    so it contains the datatypes of its elements as
    elements 2..N of the ``tuple`` datatype. i.e. change:

    .. list-table::
        :widths: 20 80
        :stub-columns: 1

        * - Datatype
          - | ``["tuple", [<datatype>, <datatype>, ...]]``

        * - Example
          - | ``["tuple", [["int"], ["string"]]]``

        * - Transport example
          - | as JSON-array:
            | ``[300,"accelerating"]``

    to:

    .. list-table::
        :widths: 20 80
        :stub-columns: 1

        * - Datatype
          - | ``["tuple", <datatype>, <datatype>, ...]``

        * - Example
          - | ``["tuple", ["int"], ["string"]]``

        * - Transport example
          - | as JSON-array:
            | ``[300,"accelerating"]``

2. rename the ``struct`` datatype
    to ``collection`` or something better expressable if not filled during transport.


Discussion
----------
preliminary discussion quickly postponed until Lutz is back again.
