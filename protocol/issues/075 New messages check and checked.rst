SECoP Issue 75: New messages check and checked (proposed)
=========================================================

Motivation
----------

For simulation purposes, an ECS might want to get parameters checked for validity,
in case the validity can not be taken from the datainfo alone.

Proposal 1: Check value messages
--------------------------------

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
~~~~~~~~~~

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

In 2023-09-26_vidconf It was decided to drop the extra messages (as they put a lot of implementation burden
on ALL SECnodes). Instead, additional commands should be implemented, which check the given value.

Details need to be discussed, but a first rough draft could as follows:


Proposal 2: check value method
------------------------------

For every parameter ``<paramname>`` which should support the client-side checking of values to be *set*,
(before they are actually set) an accompanying command ``<paramname_check>`` is to be defined.
The single argument of such a function is of the same datainfo as the parameter to be checked.

The result is an optional struct with the following members:

- ``result``: an mandatory enum with the following members:
  - ``Ok<0>``: The value is acceptable and will be accepted if set by the ECS.
  - ``limited<1>``: The value is outside (currently) acceptable limits, i.e. may work under other circumstances.
  - ``Out of range<2>``: The value is outside datainfo limits, may never work
- ``closest_valid``: an optional value according to the datainfo of ``<paramname>``, giving the closest acceptable value which could be used instead. Actual value is implementation specific.


.. note:: the ``result`` member ``limited`` is meant for cases where the actual limits depend on other modules.

Example
~~~~~~~

A vector field magnet with 3 components may have a ``value``/``target`` parameter
complying to the datainfo of

.. code::

    {"type": "array",
     "min": 3,
     "max": 3,
     "members": {"type": "float",
                 "min": -1.0,
                 "max": 1.0,
                 "unit": "T"}
    }

whilst only beeing capable of providing a field magnitude of 1.2T in total.
i.e. ``target`` = [1.0, 0.0, 0.0] works, while [1.0, 1.0, 0] won't work as the magnitude would be 1.4T and hence > 1.2T.

a check function ``target_check`` may then have the following datainfo:

.. code::

    {"type": "command",
     "argument": {"type": "array",
                  "min": 3,
                  "max": 3,
                  "members": {"type": "float",
                              "min": -1.0,
                              "max": 1.0,
                              "unit": "T"}
                  },
     "result": {"type": "struct",
                "members": {"result": {"type": "enum",
                                       "members": {"Ok": 0,
                                                   "limited": 1,
                                                   "Out of range": 2}
                                      },
                            "closest_valid": {"type":"array",
                                              "min": 3,
                                              "max": 3,
                                              "members": {"type": "float",
                                                          "min": -1.0,
                                                          "max": 1.0,
                                                          "unit": "T"}
                                             },
                            "optional": ["closest_valid"]}
                }
    }

upon issuing a command request to this function, the SECnode could then reply like this:

.. code::

  > do mf:target_check [1.0, 0.0, 0.0]
  < done mf:target_check [{"result": 0}, {"t":1505396348.876}]

  > do mf:target_check [1.0, 1.0, 0.0]
  < done mf:target_check [{"result": 2, "closest_valid": [0.84, 0.84, 0]}, {"t":1505396348.876}]




.. note:: It makes no sence to define a check funtion for a readonly parameter or a function!

Discussion
~~~~~~~~~~

None yet.
