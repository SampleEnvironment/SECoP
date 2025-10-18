SECoP Issue 81: dynamic descriptive data (proposed)
===================================================

Motivation
----------

There are some edge cases, where some elements of descriptive data may change
during the time a SEC node is running. Even if this changing data is not crucial for
running the experiment, there should be a way that these changes are reported
via SECoP, without the need of closing the connection and reading the description
again.

Proposal
--------

Some descriptive data properties may be made dynamic by referring to a
parameter. For this "@" is appended to the property name and the property
value is the name of a parameter on the same module carrying the value
of the property. Examples:


datainfo unit
.............

The use case of the power supply showed that there is a nice way to avoid chaning
units, namely creating separate modules per physical unit, and then instead of changing
the unit of one central module activate the module with the used unit and deactivate
the others.

However, if for any reason this is not wanted, we may create a parameter carrying
the unit, and refer to this in the module description:

.. code:: json

    ....
    "parameters": {
        "value": {
            "datainfo": {
                "type": "double",
                "unit@": "unit"
            }
            ...
        }
        "unit": {
            "datainfo": {
                "type": "string"
            }
            ...
        }
    }


A client might be aware of this mechanism and correctly take the value unit from
the unit parameter. A client which is not aware of this, will just treat the value
as unit less, but the user still might guess that the meaning unit parameter is
the unit of the value.

Remark: the name of the parameter may be choosen freely, however, it makes sense
to have a convention for the main unit (value / target).


datainfo min / max
------------------

Instead of the postfix rule ("_max" postfix) for user limits, we make the
rule explicit via the "max@" datainfo 'property'.

.. code:: json

    ....
    "parameters": {
        "value": {
            "datainfo": {
                "type": "double",
                "min": 0,
                "max": 15,
                "max@": "target_max"
            }
            ...
        }
        "target_max": {
            "datainfo": {
                "type": "double"
            }
            ...
        }
    }

We might or might not impose naming rules for these parameters.
