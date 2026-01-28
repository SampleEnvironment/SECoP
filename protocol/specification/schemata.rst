.. _schemata:

Schema definitions for SECoP interfaces and semantics
=====================================================

See also :ref:`rfc-102` and :ref:`rfc-103`.

The SECoP data model described in this specification is comprised of many
different kinds of entities, such as modules, parameters and properties.

Each of the pre-defined entitites specifies certain semantics -- for example,
what does the interface class `Readable` mean and which are its required and
optional parameters and commands.

The goal of this part of the specification is to formalize these definitions in
a more machine-processable form, while not losing human readability and too much
flexibility.  This in turn enables automatic validation and checking of a SECoP
node's interface against the specification.

The entities described are:

- :ref:`systems`
- :ref:`interface-classes` and :ref:`features`
- :ref:`Parameters, parameter postfixes, commands <accessibles>`
- :ref:`Properties <descriptive-data>`
- :ref:`data-types`

The definitions for standard entities should be accessible in a central
repository.  SEC nodes should link to their respective specifications in order
to enable three things:

- The implementors of clients can get a description of the functionality of
  these interface classes.
- A client can use the extended description to provide further functionality.
- The structure of the SECNode can be verified to follow the interface.


YAML definitions
----------------

Definitions are given in the YAML format, in a form similar to `object
descriptions in Kubernetes
<https://kubernetes.io/docs/concepts/overview/working-with-objects/>`_.
Multiple definitions can be placed in the same file, using the "document
separator"s of YAML.

Each entity has a few common fields:

``kind``
  The kind of entity, one of ``Repository``, ``System``, ``Interface``,
  ``Feature``, ``Parameter``, ``ParameterPostfix``, ``Command``, ``Property``,
  ``Datainfo``.
``name``
  The entity's unique name.
``version``
  The revision of this definition, as a simple integer.
``link``
  A URL linking to the description of the entity.  Optional.
``description``
  A human-readable description.

Then, depending on the ``kind``, different keys can be present:

**For repositories:**

A repository defines a collection of entities, such as "SECoP 1.0" or "Rock-IT
SECoP extensions".

``files``
  A list of other YAML file paths, relative to this file, in which the entities
  making up the repository can be found.
``systems``, ``interfaces``, ``features``, ``parameters``, ``postfixes``, ``commands``, ``datainfo``
  Lists of references_ to entities that are part of the repository.
``properties``
  Dictionary of lists of references_ to properties in the repository, keyed
  by the entity they can appear on: ``SECNode``, ``System``, ``Module``,
  ``Parameter``, ``Command``

**For interface classes and features:**

``base``
  Reference to the base interface/feature this one is derived from.
``properties``
  References_ to the properties that are required/allowed on this entity
  (depending on their "optional" setting).
``parameters``, ``commands``
  References_ to the accessibles that are required/allowed on this entity.

**For parameters:**

``readonly``
  Boolean, if the parameter should be readonly.
``datainfo``
  Specification of the parameter's datainfo_.
``properties``
  References_ to the properties that are possible on this entity.
``optional``
  Boolean, if the parameter is by default optional when added in
  interfaces/features.

**For parameter postfixes:**

``readonly``
  Boolean, if the parameter should be readonly.
``datainfo``
  Specification of the parameter's datainfo_.
``properties``
  References_ to the properties that are possible on this entity.

**For commands:**

``argument``
  The list of argument datainfo_\s, or "none".
``result``
  The return value datainfo_, or "none".
``properties``
  References_ to the properties that are possible on this entity.
``optional``
  Boolean, if the command is by default optional when added in
  interfaces/features.

**For properties:**

``dataty``
  Specification of the property's `JSON type`_.
``optional``
  Boolean, if the property is by default optional.
``value``
  If present, forces the property's value to be this - this is mainly
  useful for Systems.

**For datainfos:**

``dataty``
  Specification of the datainfo's `JSON type`_ (i.e. transport layer).
``dataprops``
  A dictionary of data properties of the datainfo specification. Each one
  can have the following properties:

  ``dataty``
    Specification of the datainfo property's `JSON type`_.
  ``optional``
    Boolean, if the property is optional.
  ``default``
    A default value.

**For systems:**

``base``
  Reference to the base system this one is derived from.
``modules``
  A dictionary of module names and their definitions.  Each item is
  either a reference to an existing interface/feature definition or a
  full inline interface definition.
``systems``
  A dictionary of subsystem names and their definitions, analogous to
  ``modules``.

Entity versions
^^^^^^^^^^^^^^^

When a new entity is proposed, the ``version`` starts at 0.  A version of 0
does not give a stability guarantee, unlike versions larger than 0.  If an
entity is accepted and introduced into the specification, the version is
defined as the major number of the specification it first appears in.
Changes to the interface afterwards require a major specification version
bump and also bump the entity's version number.

Example
^^^^^^^

As an example, a YAML description for some standard entities would look like
this:

.. code:: yaml

    ---
    kind: Parameter
    name: target
    version: 1
    datainfo: any
    readonly: false
    description: |
      The target value for the module. By setting this parameter, a move
      operation is started.

    ---
    kind: Command
    name: stop
    version: 1
    argument: none
    result: none
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
      - target:
          # Refer to this entity to get the definition of this parameter.
          definition: target:1
          # A more specific description can be given in addition to the one already
          # provided in the "definition" entity above.
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
      - stop:1

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
      - offset:1

Example for a complete system that describes a simple power supply inspired by
:issue:`078 Interacting Modules - use case power supply`:

.. code:: yaml

    ---
    kind: Property
    name: quantity
    version: 1
    datainfo: string
    optional: true
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
          - quantity:
              definition: quantity:1
              description: Must be set to "current".
        parameters:
          # This parameter is already defined by Drivable, but the required
          # datainfo is made more concrete by this definition.
          - value:
              datainfo:
                type: double
                unit: A
          # This parameter is completely specific to this module.
          - voltage_limit:
              description: |
                Compliance voltage applied when supply is in current mode.
              datainfo:
                type: double
                unit: V
              optional: true
          - power_limit:
              description: |
                Power limit applied when supply is in current mode.
              datainfo:
                type: double
                unit: W
              optional: true
          - control_active:
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
          - value:
              datainfo:
                type: number
                unit: Ohm
        properties:
          - quantity:
              definition: quantity:1
              description: Must be set to "resistance".


References
^^^^^^^^^^

A reference to another entity is one of two things:

- A string, which specifies the entity name and version separated by a colon,
  such as ``"Readable:1"``.

- A dictionary that inlines the entity, with a ``definition`` key that
  references an existing entity as ``name:version`` and adds/overrides other
  keys, most commonly the ``description`` to make it more specific.

  See the example above for how to use this.


Datainfo
^^^^^^^^

``datainfo`` entries are either strings (the name of the datainfo entity) or
dictionaries with a key ``type`` (the name of the datainfo entity) and all
dataprops of the respective datainfo.

``"any"`` is allowed for unspecified datainfos.


JSON type
^^^^^^^^^

In ``dataty`` entries, you can specify the JSON type:

- Unspecified: ``dataty: any``
- Boolean: ``dataty: bool``
- String: ``dataty: string``
- Number: ``dataty: number``
- Integer: ``dataty: int``

- Array (JSON array)::

    dataty:
      type: array
      members: <dataty>  # the dataty of array members

- Tuple (JSON array)::

    dataty:
      type: tuple
      members: [<dataty>, ...]  # list of datatys

- Struct (JSON object)::

    dataty:
      type: struct
      members:
        membername: <dataty>
        ...
      optional: [...]  # list of optional member names

Special cases:

- Any datainfo: ``dataty: datainfo``
- Same type as the parent accessible: ``dataty: parent``


.. _schema-links:

Linking to schemata
-------------------

A schema URL is of the format: ``https://.../file.yaml``, referencing a YAML
file written according to this chapter.

In the SEC node descriptive data, the optional property `schemata` is a list of
schema URLs.  All given URLs should be parsed by interested consumers (such as
clients or validators).  The entities specified in all ``Repository``\s they
contain are merged into the set of entities against which an automatic
validation is run.

This means that once `schemata` is present, all needed Repositorys needed to
validate all systems, modules, parameters etc. should be linked there.


Available repositories
----------------------

The core SECoP repositories for the different specification versions
are maintained in the SECoP repository at

https://github.com/SampleEnvironment/SECoP/tree/master/schema

To specify a certain SECoP version for a node, at least one of these URLs must
be present:

- https://raw.githubusercontent.com/SampleEnvironment/SECoP/refs/heads/master/schema/version-1.0.yaml
- https://raw.githubusercontent.com/SampleEnvironment/SECoP/refs/heads/master/schema/version-1.1.yaml
- https://raw.githubusercontent.com/SampleEnvironment/SECoP/refs/heads/master/schema/version-2.0.yaml
- or upcoming later versions
