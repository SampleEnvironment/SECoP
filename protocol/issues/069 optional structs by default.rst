SECoP Issue 69: optional structs by default (under discussion)
==============================================================

Motivation
----------

Changing single elements of a struct should be simplifiedm there is no
reason why it should be prohibited. However, for the argument of s ``do``
command, mandatory members are useful.

Proposal
--------

When no list of optional members are given in the datainfo of
a struct, all members are optional. To indicate that all members are
mandatory, ``òptional`` must be an empty array.

Discussion
----------



Decision
--------

