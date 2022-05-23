SECoP Issue 72: features (proposed)
===================================

Motivation
----------

Features allow the ECS to detect if a SECoP module support a certain functionality.
A features typically needs some predefined accessibles and/or module properties to be present.


Proposal
--------

A module property *features* lists the supported features of a module. Example:

Example
-------

'features': ['HasOffset', 'HasLimits]
