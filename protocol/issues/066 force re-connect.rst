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

An additional reply should be defined (for now called 'error_closed').
Upon reception of this, a client should dump it's internal information about the connection and
re-init the connection from its side, as if it was freshly initiated, including reading the description.
The client *MAY* also just close the network connection and re-open it.

Also, if a client initiates a connection, the identification message (``*IDN?``)
assures that the state of the connection is reset (= all subscriptions deactivated).

Discussion
----------

Open point: is a SEC node, connected via TCP/IP, allowed to behave like on a serial line,
especially

1. send an ``error_closed`` message when activated?
2. reply with  an ``error_closed`` message, when the description has changed

instead of closing the connection. The only sensible reaction of the client should
would then be to close the connection.

Decision
--------

The following has to be inserted after the first paragraph of the
`ìdentification`` message in section 2.2 Message intents:

    In addition, the identification message sets the connection to a fresh state,
    e.g. all updates are deactivated.


The following message is to be added to the specification:

    Description Changed
    ~~~~~~~~~~~~~~~~~~~

    If the SECoP communication is performed over a serial line, the following
    message is needed to tell a client, that the node description has changed.

    In asynchronous mode (activated state), this message is sent immediately,
    in synchronous mode as a reply on all messages until the connection is
    reset by an identification message (``*IDN?``).

    If the connection can be closed by the SEC-node like for a TCP/IP connection,
    error_update is not needed, but does no need to be sent.

    Example:

    .. code::

      > read t1:value
      < error_closed
      > *IDN?
      < ISSE&SINE2020,SECoP,V2019-09-16,v1.0
      > read t1:value
      < reply t1:value [295.13,{"t":1505396348.188}]


