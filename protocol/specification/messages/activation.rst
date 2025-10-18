``activate``, ``deactivate``: Control events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. message:: [request] activate [<module>]
             [reply] active [<module>]
             [reply] error_activate [<module>] <error-report>

    This request triggers the SEC node to send the values of all its modules and
    parameters as `update` messages (initial updates).  When this is finished, the
    SEC node must send an `active` reply (*global activation*).

    This initial update is to help the ECS establish a copy of the
    'assumed-to-be-current' values.  The values transferred are not necessarily read
    fresh from the hardware, check the timestamps!

    .. note:: An ECS MUST be able to handle the case of an extra update occurring
              during the initial phase, i.e. it must handle the case of receiving
              more than one update for any valid specifier.

    A SEC node might accept a module name as second item of the message
    (*module-wise activation*), activating only updates on the parameters of the
    selected module.  In this case, the "active" reply also contains the module
    name.

    A SEC node not implementing module-wise activation MUST NOT send the module name
    in its reply to an module-wise activation request, and MUST activate all modules
    (*fallback mode*).

.. message:: [request] deactivate [<module>]
             [reply] inactive [<module>]

    A parameterless message.  After the "inactive" reply no more updates are
    delivered if not triggered by a read message.

    Example:

    .. code::

        > deactivate
        < update t1:value [295.13,{"t":1505396348.188388}]
        < inactive

    .. admonition:: Remark

        The update message in the second line was sent before the deactivate message
        was treated.  After the "inactive" message, the client can expect that no
        more untriggered update message are sent, though it MUST still be able to
        handle (or ignore) them, if they still occur.

    The deactivate message might optionally accept a module name as second item of
    the message for module-wise deactivation.  If module-wise deactivation is not
    supported, the SEC node should ignore a deactivate message which contains a
    module name and send an `error_deactivate` reply.  This requires the ECS being
    able to handle update events at any time!

    It is not clear if module-wise deactivation is really useful.  A SEC node
    supporting module-wise activation does not necessarily need to support
    module-wise deactivation.
