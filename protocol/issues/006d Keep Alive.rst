SECoP Issue 6: Keep Alive (under discussion)
============================================

For a SECoP server, in order to detect that a client connection is dead,
it might close a connection with no messages within a defined period of time.

The discussed mechanism is:

The SECoP client has to set the connection parameter 'keepalive' to a value
representing the number of seconds it will send 'ping' (or other) messages.
The SECoP server can close connections with no messages for a time period
well above this value (more than 10% higher).

Opinions
--------

Markus proposes to mention the 10 % in the specification.
It should also be mentioned, that for keeping the connection alive
any message might be sent instead of the ping message.


Decision
--------
