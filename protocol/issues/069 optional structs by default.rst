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

As this is breaking backwards compatibility, it was decided to keep
the existing semantics, but add in addition to indicate all members
optional by ``optional=true``

Decision
--------

Decided as above on the 2022-01-25 meeting. Proposed change:

optional data property
~~~~~~~~~~~~~~~~~~~~~~
``"optional"``:
    The names of optional struct elements. When "optional" is omitted, all struct elements are mandatory.
    ``optional=[]`` indicates that all elements are mandatory, the special value ``optional=true`` indicates
    that all members are optional.

    In 'change' and 'do' commands, the ECS might omit these elements,
    all other elements must be given.
    The effect of a 'change' action with omitted elements should be the same
    as if the current values of these elements would have been sent with it.
    The effect of a 'do' action with omitted elements is defined by the implementation.

    In all other messages (i.e. in replies and updates), all elements have to be given.
