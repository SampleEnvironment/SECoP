SECoP Issue 19 Set SEC Node clock over SECoP
============================================

Proposal
--------

Provide a mean to set the SEC-nodes clock from the ECS side.

Discussion
----------

In general there may be several clients - each one may try to set the SEC Node time
to its own clock. Only if we consider that all clients have synchronous clocks, then
it is not a problem.

In SECoP, there is the possibility to use relative timestamps. Instead of setting the time,
the ECS can use the ping message to get the relative time and then adjust further timestamps
accordingly.

As we once agreed, that the complexity should be moved more to the ECS side, Markus
suggests that relative timestamps should be used in case of a SEC node with no
synchronized clock, and no standard of clock setting over SECoP should be provided.


