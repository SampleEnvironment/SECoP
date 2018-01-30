SECoP Issue 18: Interface classes (under discussion)
====================================================

Actually, only 3 interface classes are defined:

* readable (having at theat the parameters *value* and *status*)
* writable (additional parameter *target*)
* drivable (additional command *stop*)

When thinking about interface classes defining more concrete type of
modules, we might possibly get a huge number of classes, as lot of
combinations of options are imaginable.

Proposal
--------

In addition to the module property *interface_class*, *features* is introduced.
A feature describes a certain functionality, possible using a number of parameters
and or commands with predefined meanings. A feature may also have dependencies
on other features or on the interface class.

Discussion
----------
Explicit listing of features seems better than guessing them from the existence of parameters.

Open question: how to figure out the difference of an unknown base class to a known base class?

Markus proposes to use features instead of interface classes.
