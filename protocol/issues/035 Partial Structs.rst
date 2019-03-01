SECoP Issue 35: Partial Structs (under discussion)
==================================================

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

.. _`Issue 21: Usage of JSON null`: 021%20Usage%20of%20JSON%20null.rst
.. _`Issue 23: Adjust datatypes`: 023%20Adjust%20datatypes.rst

Proposal
--------
Allow partial ``change`` of struct-typed values, by specifying only the to be changed elements.
The reply MUST contain the complete value (i.e. not a partial struct)
Also allow ``do`` with partial struct data.
Still, only the elements referenced in the datatype are allowed, but we should allow to
not have to specify ALL of them.

To implement this, the datatype descriptor for `struct` should be extended by a list of names (a JSON-array), naming the optional struct elements (as strings).
SEC-Nodes not supporting this feature will only use the mapping and NOT provide that list of names.
(They may, however, put an empty list there which should be interpreted the same.)
ECS not knowing about the optionality of struct elements will always transfer everything, so compatibility is not harmed.


Discussion
----------
topic brought up several times, discussion usually goes into the direction of:
we'll specify if we get the use case....

Since commands now only have single argument, one use case is actually 'overloading'
of commands by selecting the subcommand via the name of the structs member.
Another is to specify only the changed p, i or d component without having to always specify all of them.

:related: `Issue 23: Adjust datatypes`_, `Issue 21: Usage of JSON null`_


video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use-cases need to be written down more clearly.
Issue is to be kept open for later inclusion.

around dec 2018 the idea of putting a list of optional elements popped up.
(this is described now above and is to be discussed.)
