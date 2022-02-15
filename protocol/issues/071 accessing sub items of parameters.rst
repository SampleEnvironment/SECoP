SECoP Issue 71: accessing sub items of parameters (closed)
==========================================================

Motivated by recent discussions as well as `SECoP Issue 20`_ a way needs to be found to be able
to access sub-items of structured data types and not always the full type.


Proposal
--------

extend the specifier in a structured way as follows:

Specifier:
    - starts with the name of a parameter (such as value/target/status/ etc...)
    - optionally followed by accessor specifications:

      - [<int>] ('opening-square-bracket', an integer number, 'closing-square-bracket')
        if the outermost datainfo is 'array', specifying to access only the numbered element
        if the specifyied number is negative or outside the actual array size, an appropriate
        error should be replied.

      - .<item_name> ('dot', name of an element) if the outermost datainfo is struct.
        if <item_name> specifes a non-existing element, an appropriate
        error should be replied.

      upon parsing this, the outermost datainfo is replaced with that of the accessed element, to support nesting

Examples
--------

specifiers:

- limits.max
- ctrlparams.p
- pidtable[3].i

requests:

- read T_reg:ctrlparams.p
- change T_reg:pidtable[3].i 50
- change T_reg:pidtable[3] {"i":50}
- change T_reg:pidtable [{},{},{},{"i":50}]


Discussion
----------

Remark by MZ:

    The question arises, if this feature should be mandatory or optional. I think it
    should be mandatory for the change command, else a client would need to implement
    a workaround when the feature is lacking. For the read command, the feature
    is not really needed. A SEC node with no writable complex datatypes would
    then not need to implement the feature, as the mandatory status parameter is readonly.
    The updates are anyway on the full parameter, and as a consequence, the 'changed'
    reply should also contain the full value of the parameter.


side-topic: as the size of an array-parameter can not (well, should not) be changed using accesses to subitems,
it is worth thinking about making this a parameter of itself, if the arraylength is intended to be changeable.


At the vidconf 2022-01-25 Markus opens again the discussion, as he does not want to implement
more complexity on the server side as needed, before a use case shows up.

Decision to close for now. If the issue raises again, no more discussion on 'how' should be needed.


.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 20`: 020%20PID%20tables.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
