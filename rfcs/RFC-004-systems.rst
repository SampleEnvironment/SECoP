- Feature: Systems as Collections of Modules
- Status: Open
- Submit Date: 2024-06-18
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Meta/Protocol
- PR:
- Version: 2.0

Summary
=======

This RFC proposes a way to define and implement "systems", which are collections
of SECoP modules provided by a SEC node that function and interact with defined
semantics.

This allows the definition of facility-specific and inter-facility standards for
control and metadata collection beyond the level of "a sensor" or "an analog
output", towards the level of "a cryomagnet".


Goal
====

All current standard SECoP interface entities (interface classes, standard
accessibles and so on) refer to a single module.

However, in most cases, it is not desirable (or even possible) to represent a
complex piece of hardware equipment by a single module.  In fact, since SECoP
encourages the use of standard interface classes like ``Readable``, the main
"process variables" of the equipment must be represented by a module's ``value``
parameter each, while other parameters are used for ancillary information or
configuration of the module.

Therefore, the notion of a "system" is introduced, which encompasses a
collection of modules that work and interact in a defined way.

As always, the definition of a system specifies the *minimum* interface
required, and may contain optional entities.  More entities can always be
introduced by each implementation.


Technical explanation
=====================

Specification
~~~~~~~~~~~~~

Like other standard entities, a system will be defined by the SECoP
specification.

It will specify both the *name* of each required module, and its *interface* (an
interface class, and/or additional accessibles).

The SEC node will have a new property that lists the systems contained in it,
with a mapping of system-given module names to actual modules in the node.

Systems can derive from other systems, and contain other systems as subgroups of
their modules.

Systems will be able to be specified in a semi-machine-readable manner using
YAML definition files, just like other interface entities.

Referencing Systems
~~~~~~~~~~~~~~~~~~~

In the SECNode descriptive data, the optional property ``systems`` is
introduced.  It is a JSON object with local system names as keys and the schema
name of the system and a mapping of module names as the value.  For a system to
be valid, all non-optional modules that are included in the system definition
have to be present.

Local system names may not clash with module names on the same SEC node.

Example:

.. code:: json

    "systems": {
        "cryo1": [
            "OrangeCryostat",
            {
                "T": "cryo1_T",
                "HeLevel": "cryo1_helevel",
            },
        ],
    },


For subsystems:

.. code:: json

    "systems": {
        "mag5t_x": [
            "PowerSupply",
            {
                "current": "magcur_x",
                "voltage": "magvolt_x"
            }
        ],
        "mag5t_y": [
            "PowerSupply",
            {
                "current": "magcur_y",
                "voltage": "magvolt_y"
            }
        ],
        "mag5t_z": [
            "PowerSupply",
            {
                "current": "magcur_z",
                "voltage": "magvolt_z"
            }
        ],
        "mag5t": [
            "VectorMagnet",
            {
                "T": "magtemp",
                "X": "mag5t_x",
                "Y": "mag5t_y",
                "Z": "mag5t_z",
            },
        ],
    },


Disadvantages, Alternatives
===========================

Disadvantages
~~~~~~~~~~~~~

More complexity in the specification.

Alternatives
~~~~~~~~~~~~


Open Questions
==============
