SECoP Issue 71: accessing sub items of parameters
=================================================

Motivated by recent discussions as well as `Issue 20`_ a way needs to be found to be able
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

to be done

side-topic: as the size of an array-parameter can not (well, should not) be changed using accesses to subitems,
it is worth thinking about making this a parameter of itself, if the arraylength is intended to be changeable.


.. _`Issue 020`: 067%20PID%20tables.rst