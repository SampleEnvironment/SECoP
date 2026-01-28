.. _descriptive-data:

Properties and descriptive data
===============================

This section explains the "descriptive data", also called "structure report",
i.e. the completely self-describing metadata sent by the SEC node in response to
a `describe` message.

The format is JSON, as all other data in SECoP.

.. note:: All names on each hierarchy level need to unique (i.e. not repeated)
          when lowercased.

Example of a complete description (JSON has been pretty-printed for clarity):

.. code:: json

    {
      "equipment_id": "example_heater",
      "description": "a basic example temperature SEC node.",
      "firmware": "ExampleSECoPFirmware",
      "modules": {
        "heater": {
          "description": "Example Heater",
          "implementation": "example.actuators.Heater",
          "interface_classes": ["Drivable"],
          "features": []
          "accessibles": {
            "value": {
              "description": "current value of the module",
              "datainfo": {"type": "double", "unit": "degC"},
              "readonly": true
            },
            "target": {
              "description": "target value of the module",
              "datainfo": {"type": "double", "unit": "degC"},
              "readonly": false
            },
            "status": {
              "description": "current status of the module",
              "datainfo": {
                "type": "tuple",
                "members": [
                  {"type": "enum",
                   "members": {"IDLE": 100, "WARN": 200, "BUSY": 300, "ERROR": 400}},
                  {"type": "string"}
                ]
              },
              "readonly": true
            },
            "stop": {
              "description": "Stop heating, stay at current temperature.",
              "datainfo": {"type": "command"}
            }
          }
        }
      }
    }


SEC node description
--------------------

The descriptive data is a JSON object with nested sub-hierarchies.  The
properties on the top level describe the SEC node.


Mandatory SEC node properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. node-property:: modules

    A JSON object with names of modules as key and JSON objects as values,
    see :ref:`module-description`.

    .. note:: Be aware that some JSON libraries may not be able to keep the
              order of the items in a JSON objects.  This is not required by the
              JSON standard, and not needed for the functionality of SECoP.
              However, it might be an advantage to use a JSON library which
              keeps the order of JSON object items.

.. node-property:: equipment_id

    Worldwide unique id of an equipment as string.  Should contain the name of
    the owner institute or provider company as prefix in order to guarantee
    worldwide uniqueness.

    Example: ``"MLZ_ccr12"`` or ``"HZB-vm4"``.

.. node-property:: description

    Text describing the node, in general.

    The formatting should follow the 'git' standard, i.e. a short headline (max
    72 chars), followed by ``\n\n`` and then a more detailed description, using
    ``\n`` for linebreaks.


Optional SEC node properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. node-property:: firmware

    Short string naming the version of the SEC node software.

    Example: ``"frappy-0.6.0"``

.. node-property:: implementor

    An optional string.  The implementor of a SEC node, defining the meaning of
    custom modules, status values, custom properties and custom parameters/commands.
    The implementor **must** be globally unique, for example ``"sinq.psi.ch"``.
    This may be achieved by including a domain name, but it does not need to be
    a registered name, and other means of assuring a globally unique name are
    also possible.

.. node-property:: timeout

    A time in seconds.  The SEC node should be able to respond within a time
    well below this value, i.e. this is a reply-timeout.  Default: 10 sec,
    *see* :issue:`004 The Timeout SEC Node Property`.

.. node-property:: systems

    A JSON object of system mappings for systems contained in this SEC node, see
    :ref:`systems`.

.. node-property:: schemata

    A list of URLs of :ref:`YAML repositories <schemata>` describing the
    structure and semantics of the node, see :ref:`schema-links`.


.. _module-description:

Module description
------------------

Mandatory module properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. mod-property:: accessibles

    A JSON object describing all the module's accessibles and their properties,
    see :ref:`accessible-description`.

    .. note:: Be aware that some JSON libraries may not be able to keep the
              order of the items in a JSON objects.  This is not required by the
              JSON standard, and not needed for the functionality of SECoP.
              However, it might be an advantage to use a JSON library which
              keeps the order of JSON object items.

.. mod-property:: description

    Text describing the module, formatted like the node property description.

.. mod-property:: interface_classes

    List of matching interface classes for the module, for example ``["Magnet",
    "Drivable"]``.


Optional module properties
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. mod-property:: features

    A list of supported :ref:`features` of a module.

    Example: ``["HasOffset"]``

.. mod-property:: visibility

    A string providing UIs with a hint for which user roles the module should be
    displayed, hidden or allow read access only.  MUST be one of the values in
    the two visibility columns.  The default is ``"www"``.

    .. table:: Possible combinations of access hints

        ================ ========== ======== ============ =============
         expert access    advanced   user     visibility   visibility
                          access     access   new style    old style
        ================ ========== ======== ============ =============
         rd/wr            rd/wr      rd/wr    "www"        "user"
         rd/wr            rd/wr      rd       "wwr"
         rd/wr            rd/wr      no       "ww-"        "advanced"
         rd/wr            rd         rd       "wrr"
         rd/wr            rd         no       "wr-"
         rd/wr            no         no       "w--"        "expert"
         rd               rd         rd       "rrr"
         rd               rd         no       "rr-"
         rd               no         no       "r--"
         no               no         no       "---"
        ================ ========== ======== ============ =============

    The 3 characters in new style form indicate the access on the levels
    "expert", "advanced" and "user", in this order.  "w" means full (read and
    write) access, "r" means restricted read only access on any parameter of the
    module and "-" means that the module should be hidden.

    * The old style notation must also be accepted by new SECoP clients.
    * A SECoP client SHOULD ignore any value not listed in the last two columns
      of above table.
    * A module with visibility "---" is meant not to be shown in a user
      interface, but might still be used by the client interface internally.

    .. note:: Access is NOT controlled on the SEC node side!  The visibility
              property is just a hint to the UI (client) what should be exposed
              to (or better hidden from) the users having different levels of
              expertise.  The client should implement the different access
              levels.

.. mod-property:: group

    A string identifier for grouping modules in the ECS.  It may contain ":"
    which may be interpreted as path separator between path components.  The
    lowercase version of a path component must not match the lowercase version
    of any module name on the same SEC node.

    .. dropdown:: Related issues

        | :issue:`008 Groups and Hierarchy`

.. mod-property:: meaning

    A JSON object with data regarding the module's meaning.  It provides
    metadata that is useful for interpreting measurement data in an automatic
    fashion.  It can have the keys ``function``, ``importance``, ``belongs_to``,
    ``link`` and ``key``, all of which are optional, with some restrictions.  A
    meaning property can also be added on the `accessible level <meaning>`.

    .. note:: In order for the meaning object to be valid, it must contain at
              least a ``"link"`` or a ``"function"`` field.

    - ``"function"`` is a string from an extensible list of predefined
      functions.

      Predefined ``"function"``\s:

      * ``"temperature"``
      * ``"temperature_regulation"`` (to be specified only if different from
        'temperature')
      * ``"magneticfield"``
      * ``"electricfield"``
      * ``"pressure"``
      * ``"rotation_z"`` (counter clockwise when looked at 'from sky to earth')
      * ``"humidity"``
      * ``"viscosity"``
      * ``"flowrate"``
      * ``"concentration"``
      * ``"ph"``
      * ``"conductivity"``
      * ``"voltage"``
      * ``"surfacepressure"``
      * ``"stress"``
      * ``"strain"``
      * ``"shear"``
      * ``"level"``

      This list may be extended later.

      ``_regulation`` may be postfixed if the quantity generating module is
      different from the relevant measuring device.  A regulation device MUST
      have an :ref:`interface class <interface-classes>` of at least
      `Writable`.

    - ``"importance"`` is an integer value in the range ``[0, 50]``.  It allows
      ordering elements with the same tuple of ``"function"`` and
      ``"belongs_to"`` by importance.

      Predefined values:

      * 10 means the instrument/beamline (Example: room temperature sensor
        always present)
      * 20 means the surrounding sample environment (Example: VTI temperature)
      * 30 means an insert (Example: sample stick of dilution insert)
      * 40 means an addon added to an insert (Example: a device mounted inside a
        dilution insert)

      Intermediate values might be used.  The range for each category starts at
      the indicated value minus 5 and ends below the indicated value plus 5.

      .. note:: This field can only be present if there is an entry for
                ``"function"``.

    - ``"belongs_to"`` is a string identifying the entity to which the module is
      linked. Setting this field forms a relation between the entity and the
      ``"function"`` field.

      Predefined entities:

      * ``"sample"``
      * ``"other"``

      .. note::

          - If not present, the default value ``"belongs_to":"other"`` is assumed.
          - This field can only be present if there is an entry for ``"function"``.

    - ``"link"`` is a link to a vocabulary, glossary or ontology.  Preferably a PID
      (Persistent Identifier) pointing to a specific entry.

    - ``"key"`` is a key (string) that selects an entry from the knowledge
      representation that ``"link"`` points to.  This mainly serves human
      readability if ``"link"`` already points to a specific entry.

      .. note::

          - This field must not be present if there is no ``"link"``.
          - If ``"link"`` does not point directly to an entry, the ``"key"``
            field is mandatory.

    Example:

    .. code:: json

        "meaning": {
           "function": "temperature_regulation",
           "importance": 20,
           "belongs_to": "sample",
           "link": "https://w3id.org/nfdi4cat/voc4cat_0000051",
           "key": "synthesis temperature"
        }

    This reads as: Regulation of the sample (``belongs_to``) temperature
    (``function``) in the surrounding sample environment (``importance``) .The
    ``key`` and ``link`` give additional metadata, saying that the regulated
    temperature is also the "synthesis temperature" of the experiment.

    Allowed key combinations in valid meaning objects::

        {function, importance, belongs_to}
        {function, importance}
        {key, link}
        {link}
        {function, importance, link}
        {function, importance, key, link}
        {function, importance, belongs_to, link}
        {function, importance, belongs_to, key, link}

    .. dropdown:: Related issues

         | :issue:`009 Module Meaning`
         | :issue:`026 More Module Meanings`

.. mod-property:: implementor

    A string giving the implementor of a module, defining the meaning
    of custom status values, custom properties and custom parameters/commands.  The
    implementor must be globally unique, for example ``"sinq.psi.ch"``.  This may
    be achieved by including a domain name, but it does not need to be a
    registered name, and other means of assuring a global unique name are also
    possible.

.. mod-property:: implementation

    A string indicating information about the implementation of the
    module, like a Python class.

    Example: ``"secop_psi.ppms.Field"``


Acquisition properties
~~~~~~~~~~~~~~~~~~~~~~

.. mod-property:: acquisition_channels

    On an `AcquisitionController`, this specifies the channel modules belonging
    to this controller.  The names of the channel modules are represented as the
    values of the JSON object.  The role of the channels are represented by the
    keys and can be used as such by an ECS.


.. _accessible-description:

Parameter and command description
---------------------------------

Mandatory properties
~~~~~~~~~~~~~~~~~~~~

.. acc-property:: description

    A string describing the parameter or command, formatted as for module
    description or node description.

.. acc-property:: datainfo

    For a parameter: Contains information on the type of data provided by the
    parameter and associated metadata, such as units.

    For a command: Contains information on the type of command arguments and
    result types, if any.

    See :ref:`data-types`.

    .. note:: Parameters and commands can be distinguished by the `datainfo`;
              the latter have a datainfo of ``{"type": "command", ...}``.


Mandatory parameter properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. acc-property:: readonly

    A boolean value.  Indicates whether this parameter may be changed by an ECS,
    or not.


Optional properties
~~~~~~~~~~~~~~~~~~~

.. acc-property:: group

    A string identifier for grouping parameters/commands in the ECS, within the
    containing module.  It may contain ":" which may be interpreted as path
    separator between path components.  The lowercase version of a path
    component must not match the lowercase version of any module name or
    accessible on the same SEC node.

    .. dropdown:: Related issues

        | :issue:`008 Groups and Hierarchy`

.. acc-property:: visibility

    A string providing UIs with a hint for which user roles the parameter or
    command should be displayed, hidden or allow read access only.  MUST be one
    of the values on the two visibility columns.  The default is ``"www"``.

    .. table::

        ================ ========== ======== ========== ============ =============
         expert access    advanced   user                visibility   visibility
                          access     access   readonly   new style    old style
        ================ ========== ======== ========== ============ =============
         rd/wr            rd/wr      rd/wr    false      "www"        "user"
         rd/wr            rd/wr      rd       false      "wwr"
         rd/wr            rd/wr      no       false      "ww-"        "advanced"
         rd/wr            rd         rd       false      "wrr"
         rd/wr            rd         no       false      "wr-"
         rd/wr            no         no       false      "w--"        "expert"
         rd               rd         rd       true       "rrr"        "user"
         rd               rd         no       true       "rr-"        "advanced"
         rd               no         no       true       "r--"        "expert"
         no               no         no                  "---"
        ================ ========== ======== ========== ============ =============

    The 3 characters in new style form indicate the access on the levels
    "expert", "advanced" and "user", in this order.  "w" means full (read and
    write) access, "r" means restricted read only access on the accessible and
    "-" means, the accessible should be hidden.

    The access for an accessible on a certain access level is determined by the
    strongest restriction for the combination of module visibility and
    accessible visibility at the given access level and the readonly flag.

    Example: A module has a visibility property of "wr-".  A parameter on this
    module with visibility "w--" should be allowed to be written only by
    experts, as the latter one is stronger.  For a readonly parameter with no
    visibility or with a visibility "rrr" it would be treated as "rr-", e.g. to
    be shown to experts and advanced clients, but not to simple users.

    * The old style notation must also be accepted by new SECoP clients.
    * With the new style notation, commands should only be executed when the
      corresponding character is a "w".
    * A SECoP client SHOULD ignore any value not listed in the last two columns
      of the above table.
    * An accessible with visibility "---" is meant not to be shown in a user
      interface, but might still be used by the client interface internally.

    .. note:: Access is NOT controlled on the SECnode side!  The visibility
              property is just a hint to the UI (client) what should be exposed
              to (or better hidden from) the users having different levels of
              expertise.  The client should implement the different access
              levels.

    .. note::

        There are redundant possibilities for expressing the same access levels,
        best practice for a SEC node is:

        - avoid explicit "w" on parameters with readonly=true
        - omit the parameter visibility, when it does not influence the result
        - consistently use the same style for all "visibility" properties

.. acc-property:: meaning

    A JSON object regarding the accessible meaning.  It has the same
    specification as the module `~mod.meaning` property.

.. acc-property:: checkable

    A boolean value indicating whether the accessible can be checked with a
    `check` message.  If omitted, the accessible is assumed to be not
    checkable (``checkable == false``), and the SEC node should reply with a
    `NotCheckable` error when a `check` message is sent.

    .. dropdown:: Related issues

        | :issue:`075 New messages check and checked`


Optional parameter properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. acc-property:: constant

    Optional, contains the constant value of a constant parameter.  If given,
    the parameter is constant and has the given value.  Such a parameter can
    neither be read nor written, and it will **not** be transferred after the
    activate command.

    The value given here must conform to the data type of the accessible.


Custom properties
-----------------

Custom properties may further augment accessibles, modules or the SEC node
description.

As for all custom extensions, their names must be prefixed with an underscore.
The meaning of custom properties depends on the implementor, given by the
`implementor <mod.implementor>` module property.  An ECS that doesn't know the
meaning of a custom property MUST ignore it.  The data type of a custom property
is not pre-defined, an ECS should be prepared to handle anything here.
