``*IDN?``: Identification
-------------------------

.. message:: [request] *IDN?
             [reply] ISSE,SECoP,<draft-date>,<version>

    The syntax of the identification message differs from other messages, as it
    is meant to be compatible with IEEE 488.2.  The identification request
    ``*IDN?`` is meant to be sent as the first message after establishing a
    connection.  The reply consists of 4 comma separated fields:

    - The first field ("manufacturer") is always ``ISSE`` (``ISSE&SINE2020`` in
      SECoP versions 1.x).

    - The second field ("product") is always ``SECoP``.

    - The third field can be left empty, or set to a date that indicates the
      pre-release draft of the specification is in use, current as of that date.
      In that case, the fourth field is the latest released version the draft is
      based on.

    - The fourth field specifies the released version of SECoP that the node
      implements.

Examples:

.. code::

    > *IDN?
    < ISSE,SECoP,,v2.0

    > *IDN?
    < ISSE,SECoP,2025-12-24,v2.0

    # Connecting to an older version, note the different "manufacturer"
    > *IDN?
    < ISSE&SINE2020,SECoP,V2019-09-16,v1.0
