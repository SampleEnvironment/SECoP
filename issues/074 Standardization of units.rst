SECoP Issue 74: Standardization of units (unspecified)
======================================================

Motivation
----------

It might be desirable to define a standard way to specify units.
The specification currently recommends SI units, but it is e.g.
not clear how to describe composed units like:

"m/s"  or  "m * s ^-1"

Discussion
----------

We should first have an idea about use cases where the ECS needs
machine readable units. If yes, we should try to find a software
library doing this already. A question is, if the machine readable
form is also human readable or may be converted to a nice human
readable form.
