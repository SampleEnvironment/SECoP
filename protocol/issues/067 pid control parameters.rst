SECoP Issue 67: pid control parameters (unspecified)
====================================================

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

Based on Option 1:

* predefined names ``pid_p``, ``pid_i``, ``pid_d``
* predefined prefix ``pid_`` (or ``_pid_`` ?) for possible extensions.

Discussion
----------

Often, pid parameters are individual items on the hardware. Also for the client, simple datatypes are is easier to handle.
On the other hand, PID parameters always belong together.

Decision
--------

To be added to the list of predefined parameters:

``"ctrlpars"``:
    A struct containing control parameters for closed loop control. The members names
    ``p``, ``i``, ``d`` are reserved for the three items in PID.
    The meaning is not exactly defined, especially ``p`` might correspond to a gain
    or a proportional band, ``i`` might be a time or the reciprocal value of a time etc.

