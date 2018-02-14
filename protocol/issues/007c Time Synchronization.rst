SECoP Issue 7: Time Synchronization (under discussion)
======================================================

As a SECoP server and a SECoP client might not run with a common clock,
the SECoP client should be able to correct for time slips.

On the meetings in Berlin and Garching in 2017 it was proposed to put this into
the standard, the exact wording has to be decided.

Agreement
---------
The kind of SEC-node clock shall be noted as node property in the descriptive data.

Proposal
--------
A SEC-node property called *clock* describes the kind of clock.

datatype: Enum(none=0, relative=1, absolute=2)

The default is *none*

Discussion
----------
It might happen that for some parameters no timestamps are delivered while
on others there are.
What should the client do when *none* is specified, but a timestamp
is delivered on a parameter? Is this a violation of the protocol, or should the
client ignore the timestamp?

Decision
--------
The ECS can easily detect if the clock is accurate enough by sending a ping
command. If the timestamp delivered by the pong message lies between the
time the ping message was sent and the pong message was received, then the
timestamp can be trusted, else the ECS might record the time shift and decide to
use relative times.
