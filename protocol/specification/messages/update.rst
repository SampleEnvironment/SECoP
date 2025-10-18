``update``: Events from the node
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. message:: [event] update <module>:<parameter> <data-report>
             [event] error_update <module>:<parameter> <error-report>

    When activated, update messages are delivered without explicit request from the
    client.  The value is a `data-report`, i.e. a JSON array with the value as its
    first element, and an JSON object containing the :ref:`qualifiers` as its second
    element.

    If an error occurs while determining a parameter, an `error_update` message
    has to be sent, which includes an :ref:`error-report` stating the problem.

Example:

.. code::

    > activate
    < update t1:value [295.13,{"t":150539648.188388,"e":0.01}]
    < update t1:status [[400,"heater broken or disconnected"],{"t":1505396348.288388}]
    < active
    < error_update t1:_heaterpower ["HardwareError","heater broken or disconnected",{"t":1505396349.20}]
    < update t1:value [295.14,{"t":1505396349.259845,"e":0.01}]
    < update t1:value [295.13,{"t":1505396350.324752,"e":0.01}]

The example shows an `activate` request triggering an initial update of two
values: ``t1:value`` and ``t1:status``, followed by the `active` reply.  Also,
an `error_update` for a parameter ``_heaterpower`` is shown.  After this, two
more updates on the ``t1:value`` show up with roughly 1 second between each.

.. note:: It is vital that all initial updates are sent, **before** the 'active'
          reply is sent!  An ECS may rely on having gotten all values.

To speed up the activation process, polling and caching of all parameters on the
SEC node is advised, i.e. the parameters should not just be read from hardware
for activation, as this may take a long time.

Another example with a broken sensor:

.. code::

    > activate
    < error_update t1:value ["HardwareError","Sensor disconnected", {"t":1505396348.188388}]}]
    < update t1:status [[400,"Sensor broken or disconnected"],{"t":1505396348.288388}]
    < active

Here the current temperature can not be obtained.  An `error_update` message
is used instead of `update`.
