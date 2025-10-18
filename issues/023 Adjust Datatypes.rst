SECoP Issue 23: Adjust Datatypes (closed)
=========================================

Motivation
----------
The current definition of the ``tuple`` datatape uses superflous nesting.
The current wording of the ``struct`` datatype is difficult.

Proposal
--------
1.  change the datatype of ``tuple``
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

3. re-order the limits consistently in all datatypes having limits (e.g. in min,max order)

4. make limits mandatory


Discussion
----------
preliminary discussion quickly postponed until Lutz is back again.

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Decisions:
 - proposal accepted with one minor change: ``["double"]`` should be able to specify ``null`` for each of its limits.
 - another change of structure in the descriptive data was also decided:
   the listing of modules/accessibles which so far used a list of 2*N elements is changed to
   a list of N 2-tuples, stronger grouping the names with the named items.
 - closing this issue.
