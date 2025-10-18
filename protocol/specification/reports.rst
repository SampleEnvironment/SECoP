Reports and values
==================

"Reports" define the structure of the different JSON data elements transferred
within SECoP :doc:`messages`.  Refer to the message details to know in what
place to expect what kind of report, or simple JSON values.

.. _data-report:

Data report
-----------

A JSON array with the value of a parameter as its first element, and a JSON
object containing the :ref:`qualifiers` for this value as its second element.

For example:

.. code:: json

    [15.0, {"t": 1723712392.1}]

.. note:: Future revisions may add additional array elements.  These are to be
          ignored for implementations of the current specification.


.. _error-report:

Error report
------------

An error report is used in an :ref:`error-reply` indicating that the requested
action could not be performed as request or that other problems occurred.  The
error report is a JSON array containing the name of one of the
:ref:`error-classes`, a human readable string and as a third element a
JSON object containing extra error information, which may include the timestamp
(as key "t") and possible additional implementation specific information about
the error (stack dump etc.).

For example:

.. code:: json

    ["HardwareError", "heater is broken", {"t": 1723812391.1, "stack": []}]


Structure report
----------------

The structure report is a structured JSON construct describing the structure of
the SEC node.  This includes the SEC node properties, the modules, their
module-properties and accessibles and the properties of the accessibles.

For details and examples see :ref:`descriptive-data`.


.. _value:

Value
-----

Values are transferred as a JSON value.

.. admonition:: Programming Hint

    Some JSON libraries do not allow all JSON values in their (de-)serialization
    functions.  Whether or not a JSON value is a valid JSON text, is
    controversial, see this `stackoverflow issue
    <https://stackoverflow.com/questions/19569221>`_ and :rfc:`8259`.

    (clarification: a *JSON document* is either a *JSON object* or a *JSON
    array*, a *JSON value* is any of a *JSON object*, *JSON array*, *JSON
    number* or *JSON string*.)

    If an implementation uses a library which can not (de-)serialize all JSON
    values, the implementation can add square brackets around a JSON value,
    decode it and take the first element of the result.  When encoding, the
    reverse action might be used as a workaround.  See also :rfc:`7493`.


.. _qualifiers:

Qualifiers
----------

Qualifiers optionally augment the value in a reply from the SEC node, and
present variable information about that parameter.  They are collected as named
values in a JSON object.

Currently 2 qualifiers are defined:

.. describe:: "t"

    The timestamp when the parameter has changed or was verified/measured (when
    no timestamp is given, the ECS may use the arrival time of the update
    message as the timestamp).  It SHOULD be given, if the SEC node has a
    synchronized time.  The format is that of a UNIX time stamp, i.e. seconds
    since 1970-01-01T00:00:00+00:00Z, represented as a number, in general a
    floating point when the resolution is better than 1 second.

    .. note:: To check if a SEC node supports time stamping, a `ping` request
              can be sent.

.. describe:: "e"

    The uncertainty of the quantity.  MUST be in the same units as the value.
    So far the interpretation of "e" is not fixed (sigma vs. RMS difference
    vs. other possibilities).

Other qualifiers might be added later to the standard.  If an unknown element is
encountered, it is to be ignored.
