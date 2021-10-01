SECoP Issue 63: enumeration of floating point values (unspecified)
==================================================================

Motivation
----------

We should find one way how to implement a datatype which can only take a fixed set of floating point values.

Proposal
--------

We want to extend the data type specification for enumerations with simple
``"values"`` for every option. All have to use the same simple data type,
which is described in ``"valuetype"``. Only ``"scaled"``, ``"double"`` and
``"bool"`` are allowed.

example:

``{"type":"enum", "members":{"auto":0,"300mW":1,"3W":2,"30W":3}, "values":{"auto":"nan","300mW":0.3,"3W":3,"30W":30}, "valuetype":{"type":"double"}}``

This is backward compatible to existing implementations, additional data
properties can be ignored by an ECS. One caveat is, that we have doubled
keys. If we do not want this, we could use a json array as value for every
option with enumeration value and numeric value as array parts. This
might break existing implementations.

Discussion
----------



Decision
--------

