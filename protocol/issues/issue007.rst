SECoP Issue 7: Time Synchronization (under discussion)
======================================================

As a SECoP server and a SECoP client might not run with a common clock,
the SECoP client should be able to correct for time slips.

The recommended mechanism is (proposal by Markus):

After connecting to a server, the client records its internal time before sending
a ping request.

If the pong request does not contain a timestamp,
the client knows, that the SEC node does not provide timestamps and
therefore it has to create timestamps on the time of reception of messages.

If the returned timestamp lies between the time the ping message was sent and the
time the pong message was received, it does not need to correct the timestamps.

If not, the average of the 'ping' time and the 'pong' time is calculated and
the difference to the received timestamp is used for correcting further
timestamps.

On the meetings in Berlin and Garching in 2017 it was proposed to put this into
the standard, the exact wording has to be decided.
