SECoP Issue 67: pid conrol parameter (unspecified)
==================================================

Motivation
----------

Control modules may need p, i and/or d parameters (or even more) to function correctly.
These are supposed to be accesssible via SECoP.
several incompatible approaches exist so far:

1. individual parameters 'p', 'i', and 'd' (possible put into the same group)
2. a tuple containing (p,i,d)
3. a struct containig 'p', 'i' and 'd' and possibly others.

We need define ONE standard way on how to handle this.
Also, predefined names for the other possible candidates are required.

Furthermore a decision if this is implemented as a feature or an interface class needs to be made.

Proposal
--------

Discussion
----------

Decision
--------

