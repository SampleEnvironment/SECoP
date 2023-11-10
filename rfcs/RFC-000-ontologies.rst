- Feature: Ontology Definitions and Repository
- Status: Open
- Submit Date: <your date>
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Meta/Protocol
- PR: <link to the pull-request>
- Implementation Tracking: -

Summary
=======

This RFC proposes a possibility to define the structure and meaning of SECoP
protocol elements at different levels, in a form that is both human readable and
machine processable.

A central repository is also proposed describing the elements defined by the
SECoP specification, providing this information to implementors.

Additional elements that are not yet in the specification, or too specialized to
be added to it, can be defined in the same way.


Goal
====

Many elements of the SECoP data model are defined in the specification. The goal
of this proposal is to formalize these definitions in a more machine-processable
form, while not losing human readability and too much flexibility.

These elements are:

- Interface classes and features
- Parameters, commands and properties
- Systems (to be specified!)

The definitions for standard elements should be accessible in a central
repository.  SECNodes should link to their respective specifications in order to
enable three things:

- The implementors of clients can get a description of the functionality of
  these interface classes.
- A client can use the extended description to provide further functionality.
- The structure of the SECNode can be verified to follow the interface.


Technical explanation
=====================

Definitions are given in the YAML format, in a form similar to object
descriptions in Kubernetes. Multiple definitions can be placed in the same file,
using the "document separator"s of YAML.

Each element has a few common fields:

``kind``
  The kind of element, one of ``System``, ``Interface``, ``Feature``,
  ``Parameter``, ``Command``, ``Property``.
``name``
  The element's unique name.
``version``
  The version of the definition, as a simple integer.
``description``
  A human-readable description. Optional.

Then, depending on the ``kind``, different keys can be present:

**For interface classes and features:**

``base``
  The base interface/feature this one is derived from.
``properties``
  The properties that are required on this element.
``parameters``, ``commands``
  The accessibles that are required on this element.

**For parameters:**

``readonly``
  If the parameter should be readonly.
``datainfo``
  Human readable explanation of the parameter's datainfo.

**For commands:**

``arguments``
  Human readable explanation of the arguments including datainfos.
``return``
  Human readable explanation of the return value and its datainfo.

**For properties:**

``applies_to``
  List of Kinds to which this property can be applied or ``SECNode``.
``datainfo``
  Human readable explanation of the property's datainfo.

**For systems:**

``modules``
  A dictionary of module names and their definitions. Each item is
  either a reference to an interface/feature definition

When a new element is proposed, the ``version`` starts at 0.  A version of 0
does not give a stability guarantee, unlike versions larger than 0.  If an
element is accepted and introduced into the specification, the version is
defined as 1. Changes to the interface afterwards bump the version number.

**XXX**

``optional``
  If present and true, this element is optional.

As an example, a YAML description for some standard elements would look like this:

.. code:: yaml

    ---
    kind: Parameter
    name: target
    version: 1
    datainfo: numeric
    readonly: false
    description: |
      The target value for the module. By setting this parameter, a move
      operation is started.

    ---
    kind: Command
    name: stop
    version: 1
    arguments: none
    return: none
    description: |
      Stop the current value-changing operation. If not driving, no effect.

    ---
    kind: Interface
    name: Writable
    version: 1
    # All accessibles from the base are "inherited".
    base: Readable:1
    description: |
      A base SECoP interface class for modules that can have their value changed,
      reporting their status in the meantime.
    parameters:
      status:
        # Refer to the element above to get the definition of this parameter.
        definition: target:1
        # A more specific description can be given in addition to the one already
        # provided in the "target" element above.
        description: ...

    ---
    kind: Interface
    name: Drivable
    version: 1
    base: Writable:1
    description: |
      A base SECoP interface class for modules whose values changes "slowly",
      so that the change can be stopped.
    commands:
      stop:
        definition: stop:1

    ---
    kind: Feature
    name: HasOffset
    version: 1
    description: |
      This feature is indicating that the value and target parameters are raw values, which
      need to be corrected by an offset. A module with the feature `HasOffset` must have
      a parameter `offset`, which indicates to all clients that values are to be converted
      by the following formulas:

        ECS value = SECoP value + offset

        SECoP target = ECS target - offset
    parameters:
      offset:
        definition: offset:1

Example for a complete system that describes a simple power supply inspired by
issue 78:

.. code:: yaml

    ---
    kind: Property
    name: quantity
    version: 1
    datainfo: string
    applies_to: [Parameter]
    description: |
      A hint of the physical quantity represented by this parameter.

    ---
    kind: System
    name: PowerSupply
    version: 1
    description: |
      A power supply consisting of current and voltage regulation modules.
      The active module can be switched with the parameter `control_active`.
    modules:
      current:
        definition: Drivable:1
        description: Controls the current.
        properties:
          # This property has a general definition, but here the description
          # defines a required value.
          quantity:
            definition: quantity:1
            description: Must be set to "current".
        parameters:
          # This parameter is already defined by Drivable, but the required
          # datainfo is made more concrete by this definition.
          value:
            datainfo: numeric, has unit Ampere
          # This parameter is completely specific to this module.
          voltage_limit:
            description: |
              Compliance voltage applied when supply is in current mode.
            datainfo: numeric, has unit Volts
            optional: true
          power_limit:
            description: |
              Power limit applied when supply is in current mode.
            datainfo: numeric, has unit Watts
            optional: true
          control_active:
            definition: control_active:1
            description: |
              If true, power supply is in current mode.
              Setting `voltage:control_active` resets this to false.
      # similar for power, voltage
      resistance:
        definition: Readable:1
        description: Readback for the measured resistance.
        optional: true
        parameters:
          value:
            datainfo: numeric, has unit Ohms.
          quantity:
            definition: quantity:1
            description: Must be set to "resistance".


Disadvantages, Alternatives
===========================

Disadvantages
-------------

The definition files must have a stable URL. URLs to a GitHub repository
should fulfill this condition, but one could think about a more generic
"stable URL" registry such as DOI if wanted.

Alternatives
------------

The parameters are specified themselves, with a new ``kind: Parameter``, and
referenced in the module element, by a ``definition:`` and an (optional)
description.  This reduces duplication, e.g. with the status parameter.

An option would be to allow both ways, either directly describing a
parameter or referencing one.  The disatvantage with this would be that files
may change when a parameter has to be factored out after the fact.


Open Questions
==============

If there are points that you know have to be discussed/solved, describe them
here, maybe with an example.
