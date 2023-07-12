- Feature: Template Repository
- Status: Open
- Submit Date: < your date >
- Type: Meta/Protocol
- PR: <link to the pull-request>
- Implementation Tracking: -

Summary
=======

A central repository with files describing the different SECoP interface classes provides information to implementors.

Goal
====

Interface classes and !system-interfaces! should be accessible in a central repository.
SECNodes should link to their respective specifications in order to enable three things:

- the implementors of clients can get a description of the functionality of these interface classes.
- a client can use the extended description to provide further functionality
- the structure of the SECNode can be verified to follow the interface

Technical explanation
=====================

A repository of YAML files that describe the ....

Each interface has a few common fields:

- kind: the kind of !iface!, one of System, Interface, Feature, Parameter, Property, Command
- version: the version of the !iface!
- name: the !iface!s unique name
- description: human-readable description. Optional.

Then, depending on the ``kind``, different keys can be present:

- datatype: On parameters, human readable explanation of the datatype.
- parameters: On modules, the list of parameters that are required on this !iface!
- base: on modules, the base !iface! this !iface! is derived from.
- ref: referencing another !iface!, for example the parameter.

Multiple !ifaces! can be put into one file, each with their own document separator.

As an example, the !spec! for the Drivable interface class would look like this:

.. code:: yaml

    ---

    kind: Parameter
    version: 1
    name: status
    datatype: Enumeration like in the spec.
    description: The status of the module.

    # Parameters value and target are omitted here
    ---

    kind: Interface
    version: 1
    name: Drivable
    base: Writable
    description: A base SECoP interface class for modules that can be read.
    parameters:
        status:
            ref: status:1
            description: The main status of the module, like drivable with BUSY
        # TODO: explicit or from base?
        value:
            ref: value:1
        target:
            ref: target:1

The !iface!s for the base spec are provided for completenesses sake, even though they are already described in the base spec.
They are provided in the file ``<...>/base.yaml``.

Disadvantages, Alternatives
===========================

Disadvantages
-------------

Is there a reason not to include this feature?

Alternatives
------------

The parameters could be specified themselves, with a new ``kind: parameter``, and be referenced in the module !specs!, by a ``ref:`` and an (optional) description.
This would reduce duplication, e.g. with the satus parameter.

A related option would be to allow both ways, either directly describing a parameter or referencing one.
The disatvantage with this would be that files may change when a parameter has to be factored out after the fact.

Open Questions
==============

If there are points that you know have to be discussed/solved, describe them here, maybe with an example.




This ``version`` is an integer, there are no major or minor versions.

When a new !interface! is proposed, the ``version`` starts at 0.
A version of 0 does not give a stability guarantee, unlike versions larger than 0.
If a !interface! is accepted and introduced into the specification, the version is defined as 1.
Changes to the interface afterwards bump the version number.
