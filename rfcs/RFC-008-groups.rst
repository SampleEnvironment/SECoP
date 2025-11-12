- Feature: Groups
- Status: Final
- Submit Date: 2018-02-13
- Authors: SECoP committee
- Type: Issue
- PR:
- Version: 1.0

Summary
=======

Definition of the `group` property for modules and accessibles.


Issue text
==========

Proposal
--------

Modules as well as parameters may have a property "group".
If the client has the possibility to group modules and/or
parameters, it should use this information for grouping.

The lowercase version of module group names must not clash
with the lowercase version of a module name.
The lowercase version of parameter group names must not clash
with the lowercase version of a parameter names on the same module.

The "group" property may contain colons (':') as separator,
in order to construct a hierarchy of more than one level.

Decision
--------

Accepted at the meeting 2018-02-13 in Grenoble.
