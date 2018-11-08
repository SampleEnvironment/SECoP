SECoP Issue 35: Partial Structs
===============================

Motivation
-----------
The current specification does not specify if a struct may be used 'incomplete',
i.e. without specifying all members described in the datatype.

There are several use cases and scenarious where this is relevant,
some of which were already discussed in `Issue 21: Usage of JSON null`_:

* partial change of a structured parameter
* commands with complex arguments

Currently, a value must always be complete, forbidding above use cases.

.. note:: renaming 'struct' may help clarifying the issue, see also `Issue 23: Adjust datatypes`_.

.. _`Issue 21: Usage of JSON null`: 021d%20Usage%20of%20JSON%20null.rst
.. _`Issue 23: Adjust datatypes`: 023p%20Adjust%20datatypes.rst

Proposal
--------
Allow partial ``change`` of struct-typed values, by specifying only the to be changed elements.
The reply MUST contain the complete value (i.e. not a partial struct)
Also allow ``do`` with partial struct data.
Still, only the elements referenced in the datatype are allowed, but we should allow to
not have to specify ALL of them.



Discussion
----------
topic brought up several times, discussion usually goes into the direction of:
we'll specify if we get the use case....

Since commands now only have single argument, the use case is actually 'overloading'
of commands by selecting the subcommand via the name of the structs member.

:related: `Issue 23: Adjust datatypes`_, `Issue 21: Usage of JSON null`_


video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use-cases need to be written down more clearly.
Issue is to be kept open for later inclusion.
