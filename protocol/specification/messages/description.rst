``describe``: Description
-------------------------

.. message:: [request] describe
             [reply] describing . <structure-report>

    These messages are normally exchanged directly after requesting `*IDN?`.
    The reply contains the "structure report", i.e. a JSON object describing the
    name of exported modules and their parameters, together with the
    corresponding properties.  This is explained in detail in
    :ref:`descriptive-data`.

The dot (second item in the reply message) is a placeholder for extensibility
reasons and may be changed in a later revision.  A client implementing the
current specification MUST ignore it.

Example::

    > describe
    < describing . {"modules":{"t1":{"interface_classes":["TemperatureSensor","Readable"],"accessibles":{"value": ...

.. note:: This reply might be a very long line, no raw line breaks are allowed
          in the JSON part.  Clients MUST implement a reasonable buffer size for
          these replies or use a streaming JSON decoder.
