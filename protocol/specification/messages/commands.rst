``do``: execute commands
~~~~~~~~~~~~~~~~~~~~~~~~

.. message:: [request] do <module>:<command> [<value>]
             [reply] done <module>:<command> <data-report>
             [reply] error_do <module>:<command> <error-report>

    Actions can be triggered with a command.  If an action needs significant time to
    complete (i.e. longer than a fraction of a second), the information about the
    duration and success of such an action has to be transferred via the `status`
    parameter.

    If a command is specified with an argument, the actual argument is given in the
    data part as a JSON value.  This may be also a JSON object if the datatype of
    the argument specifies that (i.e. the type of the single argument can also be a
    struct, tuple or an array, see :ref:`data-types`).  The types of arguments must
    conform to the declared datatypes from the datatype of the command argument.

    A command may have a return value, which may also be structured.  The "done"
    reply always contains a `data-report` with the return value.  If no value is
    returned, the data part is set to "null".  The "done" message should be returned
    quickly, the time scale should be in the order of the time needed for
    communications.  Still, all side-effects need to be realized and communicated
    before sending the `done` message.

    .. note::

        If a command does not require an argument, an argument MAY still be
        transferred as JSON null.  A SEC node MUST accept and treat the following
        two messages the same:

        - ``do <module>:<command>``
        - ``do <module>:<command> null``

        An ECS SHOULD only generate the shorter version.

Example:

.. code::

    > do t1:stop
    < done t1:stop [null,{"t":1505396348.876}]

    > do t1:stop null
    < done t1:stop [null,{"t":1505396349.743}]
