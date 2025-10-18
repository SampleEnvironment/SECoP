UDP discovery protocol
======================

Due to the flexibility of the SECoP node system, it is quite likely for multiple
SEC nodes to be running on a single host.  Therefore, it is not sufficient to
connect to just a single "commonly known" port to discover all available SECoP
device on that host.

All SEC nodes that wish to support autodiscovery open a UDP socket listening on
port 10767.  Upon receiving a packet on this port, they reply with a packet
containing the required information to connect to this node.

Format of the packets
---------------------

From client to nodes
    The contents of the broadcast packet sent from the client should be a JSON
    object containing the following keys:

    - ``SECoP`` set to ``discover``.

    Further keys in the object can be specified later.

From node to client
    Receiving a correctly formatted discovery packet, the node should reply to
    the sender address with a JSON object containing the following keys:

    - ``SECoP`` set to ``node``.
    - ``port`` set to the TCP port under which the node is reachable.
    - ``equipment_id`` set to the ``equipment_id`` property of the SEC node.
    - ``firmware`` set to the ``firmware`` property of the SEC node.
    - ``description`` set to the ``description`` property of the SEC node,
      shortened if necessary to keep to safe UDP packet size.

    If the node is reachable on several ports, it can send the packet once per
    port.

    Further keys in the object can be specified later.

Node self-announcement
----------------------

Furthermore, a SEC node can self-announce by broadcasting UDP packets to port
10767 on its own, preferable once at startup or in large intervals.  The content
is exactly the same as in the "node-to-clients" reply packet.

Example
-------

Here, whitespace is added for readability.  In practice, the JSON encoder should
be configured to omit them to leave maximum space for the content.

From client to nodes:

.. code:: json

    {"SECoP": "discover"}

From node to client, or for self-announcement:

.. code:: json

    {"SECoP": "node",
     "port": 14932,
     "equipment_id": "mlz_ccr12",
     "firmware": "frappy",
     "description": "A cryostat with pulse tube cooler"}


Implementation hints
--------------------

- On Linux, the ``SO_REUSEPORT`` socket option needs to be set to 1 to enable
  multiple sockets binding the same port on one host.  Received broadcast
  packets will be delivered to all of them simultaneously.

- The maximum safe UDP packet size is 508 bytes.  Therefore, the combined length
  of the strings equipment_id, firmware and description can be 430 bytes.  The
  first two should be transported entirely, and the description shortened as
  necessary.


See also :ref:`rfc-005`.
