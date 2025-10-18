``update``: Events from the node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. message:: [event] update <module>:<parameter> <data-report>
             [event] error_update <module>:<parameter> <error-report>

    When activated, update messages are delivered without explicit request from the
    client.  The value is a :ref:`data-report`, i.e. a JSON array with the value as its
    first element, and an JSON object containing the :ref:`qualifiers` as its second
    element.

    If an error occurs while determining a parameter, an `error_update` message
    has to be sent, which includes an :ref:`error-report` stating the problem.

Example::

    > activate
    < update t1:value [295.13,{"t":150539648.2,"e":0.01}]
    < update t1:status [[400,"heater broken or disconnected"],{"t":1505396348.3}]
    < error_update t1:_heaterpower ["HardwareError","heater broken or disconnected",{"t":1505396349.2}]
    < active
    < update t1:value [295.14,{"t":1505396349.3,"e":0.01}]
    < update t1:value [295.13,{"t":1505396350.3,"e":0.01}]

The example shows an `activate` request triggering an initial update of two
values, ``t1:value`` and ``t1:status``, followed by the `active` reply.  Also,
an `error_update` for a (custom) parameter ``_heaterpower`` is shown.  After
this, two more updates on the ``t1:value`` show up with roughly 1 second between
each.

.. note:: It is vital that all initial updates are sent *before* the 'active'
          reply!  An ECS may rely on having gotten all values.

To speed up the activation process, polling and caching of all parameters on the
SEC node is advised, i.e. the parameters should not just be read from hardware
for activation, as this may take a long time.

Another example with a broken sensor::

    > activate
    < error_update t1:value ["HardwareError","Sensor disconnected", {"t":1505396348.2}]}]
    < update t1:status [[400,"Sensor broken or disconnected"],{"t":1505396348.3}]
    < active

Here the current temperature can not be obtained.  An `error_update` message is
used instead of `update`.
