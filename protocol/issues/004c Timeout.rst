SECoP Issue 4: Timeout (closed)
===============================

For a SECoP client, in order to detect that a connection to a SECoP server is dead,
'ping' messages can be sent. When no 'pong' message is received within a specified
time, then the client can consider the connection as dead.

If the server does not specify a the SEC node property 'timeout', the timeout
is assumed to be 3s.

Opinions
--------

On the meeting in Garching (2017-09-14) it was proposed to fix this a standard.


Decision
--------

It is assumed that when a SEC node is not replying within *timeout*
seconds, the client may assume that the SEC node is dead or stuck.

If *timeout* is not specfied as a SEC node property. it is assumed to
be 10 seconds.
