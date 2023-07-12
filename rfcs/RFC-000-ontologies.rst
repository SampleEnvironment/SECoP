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

- Interface classes
- Features
- Parameters
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
  ``Parameter``, ``Command``.
``name``
  The element's unique name.
``version``
  The version of the definition, as a simple integer.
``description``
  A human-readable description. Optional.

Then, depending on the ``kind``, different keys can be present:

``datatype``
  On parameters, human readable explanation of the datatype.
``arguments``
  On commands, human readable explanation of the arguments.
``return``
  On commands, human readable explanation of the return value.
``parameters``, ``commands``
  On interfaces/features, the accessibles that are required on this element.
``base``
  On interfaces/features, the base interface/feature this one is derived from.

As an example, the YAML for the Writable interface class would look like this:

.. code:: yaml

    ---
    kind: Parameter
    name: target
    version: 1
    datatype: numeric
    description: |
      The target value for the module. By setting this parameter, a move
      operation is started.
    readonly: false

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

When a new element is proposed, the ``version`` starts at 0.  A version of 0
does not give a stability guarantee, unlike versions larger than 0.  If an
element is accepted and introduced into the specification, the version is
defined as 1. Changes to the interface afterwards bump the version number.


Disadvantages, Alternatives
===========================

Disadvantages
-------------

Is there a reason not to include this feature?

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
