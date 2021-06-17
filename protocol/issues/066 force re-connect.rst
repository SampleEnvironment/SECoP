SECoP Issue 66: force re-connect (proposed)
===========================================

Motivation
----------

Sometimes situations arise, where a client should be forced to re-read the description.
This can be done by closing the network connection already, but has two problems:

1. a sec node on a serial line can not close the connection
2. a client may not re-read the description upon re-read
3. just closing a network connection is not so nice and may take (unneededly) longer
   than just re-reading the description.

Proposal
--------

An additional reply should be defined (for now called 'close').
Upon reception of this, a client should dump it's internal information about the connection and
re-init the connection from its side, as if it was freshly initiated, including reading the description.
The client *MAY* also just close the network connection and re-open it.

Also, if a client initiate a connection, it *MUST* first check the identity of the secnode (via '*IDN?'),
then fetch the description and only after this start interacting with the SEC-node using the other defined messages.
A SEC-node, however, shall upon reception of '*IDN?' drop its internal state about the connection,
effectively 'deactivate'-ing all subscriptions (essential for serial lines).

Discussion
----------

open points:

1. can 'close' be send anytime by the sec-node, or only as a reply to (any) request?
2. can the client also send 'close' to the secnode? (no need for clearing state at '*IDN?', as this
   is then done at receiption of 'close', but: what should the SEC-node send as reply? (if any)

Decision
--------

