SECoP Issue 4: Timeout (under discussion)
=========================================

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
