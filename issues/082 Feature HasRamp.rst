SECoP Issue 82: Feature HasRamp (proposed)
==========================================

Motivation
----------

In the vidconf 2023-09-26 the following were proposed after some initial discussion.

Proposal
--------

An additional Feature ``HasRamp`` is to be specified with the following properties:

- a mandatory parameter ``ramp`` of a numeric datainfo and an unit of <unit of target>/minute.
- a mandatory parameter ``ramp_enable`` which is a (possibly constant) boolean,
  indicating if the ramp is in effect, or not.


The ``ramp_enable`` may be constant in cases, where the ramp can't be switched off (certain magnets).
This was preferred in the discussion over an optional parameter, because it is more explicit.

Discussion
~~~~~~~~~~

The proposal was discussed as-is in vidconf-2023-09-26. no further discussion yet.
