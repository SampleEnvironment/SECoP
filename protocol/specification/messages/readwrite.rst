``read``, ``change``: Read and write parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. message:: [request] read <module>:<parameter>
             [reply] reply <module>:<parameter> <data-report>
             [reply] error_read <module>:<parameter> <error-report>

    With the read request message, the ECS may ask the SEC node about a reasonable
    recent 'current' value.  In most cases this means that the hardware is read to
    give a fresh value.  However, there are use cases where either an internal
    control loop is running anyway in which case it is perfectly fine to return the
    internally cached value.  When it can take a long time to actually obtain a
    fresh value, it is also acceptable to return the most recently obtained value.
    In any way, the timestamp qualifier should indicate the time the value was
    **obtained**.

Example:

.. code::

    > read t1:value
    < reply t1:value [295.13,{"t":1505396348.188}]
    > read t1:status
    > reply t1:status [[100,"OK"],{"t":1505396348.548}]


.. message:: [request] change <module>:<parameter> <value>
             [reply] changed <module>:<parameter> <data-report>
             [reply] error_change <module>:<parameter> <error-report>

    The change value message contains the name of the module or parameter and the
    value to be set.  The value is JSON formatted.  As soon as the set-value is read
    back from the hardware, all clients, having activated the parameter/module in
    question, get an "update" message.  After all side-effects are communicated, a
    "changed" reply is then send, containing a `data-report` of the read-back value.

    .. admonition:: Remarks

        * If the value is not stored in hardware, the "update" message can be sent
          immediately.
        * The read-back value should always reflect the value actually used.
        * A client in async mode may get an `update` message before the
          `changed` message, both containing the same data report.

    Example on a connection with activated updates.  Qualifiers are replaced by
    ``{...}`` for brevity here.

    .. code::

        > read mf:status
        < reply mf:status [[100,"OK"],{...}]
        > change mf:target 12
        < update mf:status [[300,"ramping field"],{...}]
        < update mf:target [12,{...}]
        < changed mf:target [12,{...}]
        < update mf:value [0.01293,{...}]

    The status changes from "idle" (100) to "busy" (300).  The ECS will be informed
    with a further update message on ``mf:status``, when the module has finished
    ramping.  Until then, it will get regular updates on the current main value (see
    last update above).

    .. note:: It is vital that all 'side-effects' are realized (i.e. stored in
              internal variables) and be communicated **before** the 'changed' reply
              is sent!

    .. XXX move this below!

    .. dropdown:: Correct handling of side-effects

        To avoid difficult to debug race conditions, the following sequence of
        events should be followed whenever the ECS wants to initiate an action:

        1) ECS sends the initiating message request (either `change` or `do`)
           and awaits the response.

        2) SEC node checks the request and if it can be performed. If not, SEC node
           sends an error-reply (sequence done).  If nothing is actually to be done,
           continue to point 4.

        3) If the action is fast-finishing, it should be performed and the sequence
           should continue to point 4.  Otherwise the SEC node 'sets' the status
           code to ``BUSY`` and instructs the hardware to execute the requested
           action.  Also an `update` status event (with the new BUSY status-code)
           MUST be sent to **ALL** activated clients (if any).  From now on, all
           read requests will also reveal a BUSY status-code.  If additional
           parameters are influenced, their updated values should be communicated as
           well.

        4) SEC node sends the reply to the request of point 2 indicating the success
           or failure of the request.

           .. note:: An error may be replied after the status was set to BUSY if
              triggering the intended action failed (communication problems?).

        5) When the action is finally finished and the module no longer to be
           considered ``BUSY``, an `update` status event to ``IDLE`` MUST be sent,
           also subsequent status queries should reflect the now no longer BUSY
           state. Of course, all other parameters influenced by this should also
           communicate their new values.

        An ECS establishing more than one connection to the same SEC node and which
        **may** process the `update` event message from point 3 after the reply of
        point 4 MUST query the status parameter synchronously to avoid the
        race-condition of missing the (possible) BUSY status-code.

        Temporal order should be kept wherever possible!


