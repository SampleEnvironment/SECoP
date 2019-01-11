SECoP Issue 50: Reserved Names
==============================

Motivation
----------

Make a list of reserved names for proposed, but not yet agreed features.

Proposal
--------

Based on the current version of the specification (secop_v2018-11-07.rst) and the
decisions of vidconf_2018-12-03 we have the following reserved names:

- qualifiers: unit
- parameters: unit, mode, loglevel, ramp, use_ramp, setpoint, time_to_target
- commands: go, abort, hold, shutdown
- properties: precision, fmtstr, export

Discussion
----------

Markus:
I see no need to put reserved names in the standard. Names of custom extensions
anyway start with an underscore, therefore we have no danger of names clashes.
If needed, we might keep a list of reserved names as an issue, for internal use within
the committee.
