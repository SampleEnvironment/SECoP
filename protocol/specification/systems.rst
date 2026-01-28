.. _systems:

Systems
=======

See also :ref:`rfc-104`.

All standard SECoP interface entities described so far (interface classes,
accessibles and so on) refer to a single module.

However, in many cases, it is not desirable (or even possible) to represent a
complex piece of hardware equipment by a single module.  In fact, since SECoP
encourages the use of standard interface classes like `Readable`, the main
"process variables" of the equipment must each be represented by a module's
`value` parameter, while other parameters are used for ancillary information or
configuration of the module.

The notion of a "system" encompasses a collection of modules that work and
interact in a defined way.

As always, the definition of a system specifies the *minimum* interface
required, and may contain optional entities.  More entities can always be
introduced by each implementation.

This allows the definition of facility-specific and inter-facility standards for
control and metadata collection beyond the level of "a sensor" or "an analog
output", towards the level of "a cryomagnet".

System mapping
--------------

Like other standard entities, standard systems can be defined by the SECoP
specification.  Currently, no such systems are defined.

A specification designates both the *name* of each required module, and its
*interface* (an interface class, and/or additional accessibles).

Custom systems can be defined by linking to a schema in the node description,
see :ref:`Schema <schemata>`.

The SEC node descriptive data has a node property that lists the systems
contained in it, with a mapping of system-given module names to actual modules
in the node.

Systems can derive from other systems, and contain other systems as subgroups of
their modules.

Referencing Systems
-------------------

In the SECNode descriptive data, the optional property `systems` is introduced.
It is a JSON object with local system names as keys and a description, the
schema name of the system and a mapping of module names as the value.  For a
system to be valid, all non-optional modules that are included in the system
definition have to be present.

Local system names may not clash with module names on the same SEC node.

Example:

.. code:: json

    "systems": {
        "cryo1": {
            "description": "A cryo",
            "system": "OrangeCryostat",
            "modules": {
                "T": "cryo1_T",
                "HeLevel": "cryo1_helevel",
            },
        },
    }


For subsystems:

.. code:: json

    "systems": {
        "mag5t_x": {
            "description": "X axis power supply",
            "system": "PowerSupply",
            "modules": {
                "current": "magcur_x",
                "voltage": "magvolt_x"
            }
        },
        "mag5t_y": {
            "description": "Y axis power supply",
            "system": "PowerSupply",
            "modules": {
                "current": "magcur_y",
                "voltage": "magvolt_y"
            }
        },
        "mag5t_z": {
            "description": "Z axis power supply",
            "system": "PowerSupply",
            "modules": {
                "current": "magcur_z",
                "voltage": "magvolt_z"
            }
        },
        "mag5t": {
            "description": "5 Tesla magnet",
            "system": "VectorMagnet",
            "modules": {
                "T": "magtemp",
                "X": "mag5t_x",
                "Y": "mag5t_y",
                "Z": "mag5t_z",
            },
        },
    }
