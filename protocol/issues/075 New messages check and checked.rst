SECoP Issue 75: New messages check and checked (proposed)
=========================================================

Motivation
----------

For simulation purposes, an ECS might want to get parameters checked for validity,
in case the validity can not be taken from the datainfo alone.

Proposal
--------

Check Value
~~~~~~~~~~~

The check value message contains the name of the module or parameter
and the value to be checked. The value is JSON formatted.
In case the transmitted value is valid for a change message, the reply must
be a ``checked`` message, with the same identifier as the ``check`` message.
The data part of the returned value composed by a null value
(or should this be a copy of the data?) and the qualifiers.
The qualifiers may contain a key ``condition`` with a string indicating that
the value might be invalid depending on values of other modules.
If possible, this should be written in the form "<module>:<parameter> < <value>".
The ``check`` message must not change anything, neither on the hardware
nor on any parameter.
In case the transmitted value is invalid, an ``error_check`` message must be
sent back. If applicable, the SEC-Node should in addition return the closest
possible valid value as additional information in the error report with
the key ``closest_valid``.

The result of a check message MUST not depend on the state of the SEC-Node.

Example with mf representing a vector magnet.

.. code::

  > check mf:target [1.0, 1.0, 2.0]
  < checked mf:target [null, {}]
  > check mf:target [1.0, 2.0, 2.5]
  < error_check mf:target ["BadValue", "value outside allowed sphere", {"closest_valid": [0.8, 1.6, 2.0]}]

Another example: ``mf`` representing the magnetic field of a cryomagnet and ``T_lambda``
representing the lambda stage temperature:

.. code::

  > check mf:target 14.9
  < checked mf:target [null, {"condition": "lambda:value < 2.5"}]


Note:

   The ``checked`` and ``check_error`` message is only sent as a reply to the ``check``
   message on connection, but not to other activated connections.


In addition ``closest_valid`` may also be returned on 'error_change' messages, especially
in case the closest valid value can not be determined from the datainfo min or max.


Discussion
----------

Markus:
    With the additional information ``closest_valid`` in the error report the problem about
    a combination of target_limits and offset on the SEC-Node side might be solved:

  > check m:target_limits [-360, 360]
  < error_check m:target_limits ["BadValue", "out of range", {"closest_valid": [-0.5, 359.5]}]

Note:
    I do not want to raise again the offset war. My use case for this is covered perfectly.
    But I can imagine that a use case will raise up later, where an interaction from a client
    might change the valid range of a parameter. This feature might help to solve it without
    need of changing the descriptive data.
