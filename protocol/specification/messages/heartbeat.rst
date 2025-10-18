``ping``: Heartbeat
~~~~~~~~~~~~~~~~~~~

.. message:: [request] ping [<token>]
             [reply] pong [<token>] <data-report>

    In order to detect that the other end of the communication is not dead, this
    heartbeat may be sent.  The second part of the message (the token) must not contain
    a space and should be short and not be re-used.  It may be omitted.  The reply
    will contain exactly the same id.

    A SEC node replies with a `pong` message with a :ref:`data-report` of a null value.
    The :ref:`qualifiers` part SHOULD only contain the timestamp (as member "t") if
    the SEC node supports timestamping.  This can be used to synchronize the time
    between ECS and SEC node.

    .. note:: The qualifiers could also be an empty JSON object, indicating lack of
              timestamping support.

    For debugging purposes, when *id* in the `ping` request is omitted, in the
    `pong` reply there are two spaces after `pong`.  A client SHOULD always send
    an id.  However, the client parser MUST treat two consecutive spaces as two
    separators with an empty string in between.

Example:

.. code::

    > ping 123
    < pong 123 [null, {"t": 1505396348.543}]

.. dropdown:: Related issues

    | :issue:`003 Timestamp Format`
    | :issue:`007 Time Synchronization`
