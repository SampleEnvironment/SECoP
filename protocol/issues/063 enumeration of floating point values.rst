SECoP Issue 63: enumeration of floating point values (closed)
=============================================================

Motivation
----------

We should find one way how to implement a datatype which can only take a fixed set of floating point values.

Proposal
--------

We want to extend the data type specification for enumerations with simple
``"values"`` for every option. All have to use the same simple data type,
which is described in ``"valuetype"``. Only ``"scaled"``, ``"double"`` and
``"bool"`` are allowed.

  discussion in video conference 2021-11-03 - this example had not
  enough supporters:

  ``{"type":"enum", "members":{"auto":0,"300mW":1,"3W":2,"30W":3}, "values":{"auto":"nan","300mW":0.3,"3W":3,"30W":30}, "valuetype":{"type":"double"}}``

  This is backward compatible to existing implementations, additional data
  properties can be ignored by an ECS. One caveat is, that we have doubled
  keys. If we do not want this, we could use a json array as value for every
  option with enumeration value and numeric value as array parts. This
  might break existing implementations.

Leave ``"enum"`` data type as it is and add another readable or writable
parameter(s) of type ``"double"``, ``"scaled"`` or ``"bool"`` with the actual
equivalent value of the enumeration.

We add a property ``"influences"`` to all of the linked parameters, which
contains the other parameter name(s) as ``array of strings``. You might omit
the module name and *single* colon of an entry and this means a parameter of
the same module. Anyway, you are also allowed to use full SECoP specifier(s).

The ECS could use this in case of synchronized view of parameters.
If one parameter is changed with an asynchronous connection, the other
parameter(s) will be updated too. For a synchronous connection, the ECS
has to poll the influenced parameters by its own. The SEC node decides,
which value is chosen, if a client wants to change an influenced numeric
parameter to a value, which does not perfectly maps to an enumeration value.

Caveats are: the ECS has to interpret the keys (strings) of the enumeration
to check, what numeric values are allowed.

See also `SECoP Issue 62: naming convention for related parameters`_ and
`SECoP Issue 65: handling of coupled modules`_ .

Discussion
----------

vidconf 2021-11-03:

  Enno points out a few surprising side-effects which may happen with sec-
  nodes without float support. Discussion about the need of the "valuetype"
  entry.

  Counter usage example is if a user tries to set a value which is not
  amongst the "values" mapping.

  Enno has two thoughts about this:

  1) Why not extend a "double" typed parameter with a property
     "restricted_values" containing a list of all allowed values.
     Markus mentions that this may be uncomfortable for the user, if the
     parameter range spans several decades. This could be handled in the ECS,
     though.

  2) A set of two 'linked' parameters: one of "enum" type (as before) and a
     "double" (or "scaled") which represents the wanted numeric representation
     value of the selected enum value. Updating one of those linked parameters
     will then have to also update the other parameter.
     Open question: indication of this linkage via an additional property
     "influences" (a list of parameter names being potentially changed by a
     change of this parameter) or via a "common name prefix/suffix"?

  Discussion continues and opinions seems to tend to favor proposal (2) with
  the property. Discussion about depending parameters and use cases.

vidconf 2021-11-30:

  Markus wants to have a link between enumeration entries and a numerical value
  for saving in data files. Setting a numeric value, the secnode should handle
  non-listed values (rounding up or down). A gui would continue to use the
  enumeration values.

  This link can be done via a ``influences`` property, instead of adding extra
  ``values`` parameter. Using two 'linked' parameters is agreed upon.

  side-topic: ``influences`` entries *may* have contain a *SINGLE* colon (':')
  to indicate 'module:parameter' style references to parameters of other modules.

Decision
--------

We keep the existing data type definition.
We add an additional property ``"influences"`` as described above.

.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 62: naming convention for related parameters`: 062%20naming%20convention%20for%20related%20parameters.rst
.. _`SECoP Issue 65: handling of coupled modules`: 065%20handling%20of%20coupled%20modules.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
