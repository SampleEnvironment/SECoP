SECoP Issue 69: optional structs by default (finalizing)
========================================================

Motivation
----------

Changing single elements of a struct should be simplified, there is no
reason why it should be prohibited. However, for the argument of a ``do``
command, mandatory members are useful.

Proposal
--------

When no list of optional members are given in the datainfo of
a struct, all members are optional. To indicate that all members are
mandatory, ``optional`` must be an empty array.

Discussion
----------

Comparing the 2 "trivial" cases" no/all members optional

Previous specification was:

- all members mandatory: no optional entry in the datainfo JSON object
- all members optional: optional=[... listing all members ...]

Proposal:
- all members mandatory: optional=[]
- all members optional: no optional entry in the datainfo JSON object

Decision
--------

Decided as above on the 2022-01-25 meeting.
