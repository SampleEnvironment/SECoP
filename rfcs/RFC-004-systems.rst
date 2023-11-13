- Feature: Systems as Collections of Modules
- Status: Open
- Submit Date: <your date>
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Meta/Protocol
- PR: <link to the pull-request>
- Implementation Tracking: -

Summary
=======

This RFC proposes a way to define and implement "systems", which are collections
of SECoP modules provided by a SEC node that function and interact in a defined
way.

This allows the definition of inter-facility standards for control and metadata
collection beyond the level of "a sensor" or "an analog output", towards the
level of "a cryomagnet".


Goal
====

All current standard SECoP interface elements (interface classes, standard
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
required, and may contain optional elements.  More elements can always be
introduced by each implementation.


Technical explanation
=====================

Specification
~~~~~~~~~~~~~

Like other standard elements like, a system will be defined by the SECoP
specification.

It will specify both the *name* of each required module, and its *interface* (an
interface class, and/or additional accessibles).

Preferably, if RFC 2 is implemented, systems will be able to be specified in a
semi-machine-readable manner using YAML definition files, just like other
interface elements.

Naming Scheme
~~~~~~~~~~~~~

In SEC nodes, the name of a module must be composed as ::

   <instance-name>_<module-name>

So for example, if a system called "power supply" specifies a module ``current``
and a module ``voltage``, and the SEC node has a power supply instance called
``lasersupply``, it needs to provide modules called ``lasersupply_current`` and
``lasersupply_voltage``.

Definition of Systems
~~~~~~~~~~~~~~~~~~~~~

If RFC 3 is implemented


Disadvantages, Alternatives
===========================

Disadvantages
-------------

More complexity in the specification.

Alternatives
------------

Instead of a fixed naming scheme, one could define the mapping from
"system-given name" to "concrete name" in the SEC node's descriptive data.
The concrete names could then be freely chosen.


Open Questions
==============
