.. _descriptive-data:

Descriptive Data
================

This section explains the "descriptive data", also called "structure report",
i.e. the completely self-describing metadata sent by the SEC node in response to
a ``describe`` message.

The format is JSON, as all other data in SECoP.

.. note:: All names on each hierarchy level need to unique (i.e. not repeated)
          when lowercased.


SEC Node Description
--------------------

.. image:: images/sec-node-description.svg
   :alt: SEC_node_description ::= '{' ( property ',' )* '"modules":' modules ( ',' property )* '}'

.. compound::

    Property:

    .. image:: images/property.svg


Mandatory SEC Node Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"modules"``
    A JSON object with names of modules as key and JSON-objects as values,
    see `Module Description`_.

    .. note:: Be aware that some JSON libraries may not be able to keep the
              order of the items in a JSON objects.  This is not required by the
              JSON standard, and not needed for the functionality of SECoP.
              However, it might be an advantage to use a JSON library which
              keeps the order of JSON object items.

``"equipment_id"``
     Worldwide unique id of an equipment as string.  Should contain the name of
     the owner institute or provider company as prefix in order to guarantee
     worldwide uniqueness.

     Example: ``"MLZ_ccr12"`` or ``"HZB-vm4"``.

``"description"``
     Text describing the node, in general.

     The formatting should follow the 'git' standard, i.e. a short headline (max
     72 chars), followed by ``\n\n`` and then a more detailed description, using
     ``\n`` for linebreaks.


Optional SEC Node Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"firmware"``
     Short string naming the version of the SEC node software.

     Example: ``"frappy-0.6.0"``

``"implementor"``
     An optional string.  The implementor of a SEC node, defining the meaning of
     custom modules, status values, custom properties and custom accessibles.
     The implementor **must** be globally unique, for example 'sinq.psi.ch'.
     This may be achieved by including a domain name, but it does not need to be
     a registered name, and other means of assuring a globally unique name are
     also possible.

``"timeout"``
     A time in seconds.  The SEC node should be able to respond within a time
     well below this value, i.e. this is a reply-timeout.  Default: 10 sec,
     *see* :issue:`004 The Timeout SEC Node Property`.


.. _module-description:

Module Description
------------------

.. image:: images/module-description.svg
   :alt: module_description ::= '{' ( property ',' )* '"accessibles":' accessibles ( ',' property )* '}'

Mandatory Module Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"accessibles"``
    A JSON object describing all the module's accessibles and their properties,
    see `Accessible Description`_.

    .. note:: Be aware that some JSON libraries may not be able to keep the
              order of the items in a JSON objects.  This is not required by the
              JSON standard, and not needed for the functionality of SECoP.
              However, it might be an advantage to use a JSON library which
              keeps the order of JSON object items.

``"description"``
    Text describing the module, formatted like the node property description.

``"interface_classes"``
    List of matching interface classes for the module, for example ``["Magnet",
    "Drivable"]``.


Optional Module Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~

``"visibility"``
    String giving a hint for UIs for which user roles the module should be
    display or hidden.  MUST be one of "expert", "advanced" or "user"
    (default).

    .. note:: This does not imply that the access is controlled.  It is just a
              hint to the UI for the amount of exposed modules.  A visibility of
              "advanced" means that the UI should hide the module for users, but
              show it for experts and advanced users.

``"group"``
    A string identifier for grouping modules in the ECS.  It may contain ":"
    which may be interpreted as path separator between path components.  The
    lowercase version of a path component must not match the lowercase version
    of any module name on the same SEC node.

    Related issue: :issue:`008 Groups and Hierarchy`

``"meaning"``
    A tuple, with the following two elements:

    1. A string from an extensible list of predefined meanings:

       - ``"temperature"`` (the sample temperature)
       - ``"temperature_regulation"`` (to be specified only if different from 'temperature')
       - ``"magneticfield"``
       - ``"electricfield"``
       - ``"pressure"``
       - ``"rotation_z"`` (counter clockwise when looked at 'from sky to earth')
       - ``"humidity"``
       - ``"viscosity"``
       - ``"flowrate"``
       - ``"concentration"``

       This list may be extended later.

       ``_regulation`` may be postfixed if the quantity generating module is
       different from the (closer to the sample) relevant measuring device. A
       regulation device MUST have an :ref:`interface class <interface-classes>`
       of at least ``Writable``.

       Related issue: :issue:`026 More Module Meanings`

    2. A value describing the importance, with the following values:

       - 10 means the instrument/beamline (example: room temperature sensor
         always present)
       - 20 means the surrounding sample environment (example: VTI temperature)
       - 30 means an insert (example: sample stick of dilution insert)
       - 40 means an addon added to an insert (example: a device mounted inside
         a dilution insert)

       Intermediate values might be used.  The range for each category starts at
       the indicated value minus 5 and ends below the indicated value plus 5.

       Related issue: :issue:`009 Module Meaning`

.. _implementor:

``"implementor"``
    An optional string giving the implementor of a module, defining the meaning
    of custom status values, custom properties and custom accessibles.  The
    implementor must be globally unique, for example 'sinq.psi.ch'.  This may
    be achieved by including a domain name, but it does not need to be a
    registered name, and other means of assuring a global unique name are also
    possible.


Accessible Description
----------------------

.. image:: images/accessible-description.svg
   :alt: accessible_description ::= '{' property+ '}'


Mandatory Accessible Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"description"``
    A string describing the accessible, formatted as for module description or
    node description.


Mandatory Parameter Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. _prop-readonly:

``"readonly"``
    A boolean value.  Indicates whether this parameter may be changed by an ECS,
    or not.

``"datainfo"``
    Contains information on the type of data provided by the accessible and
    associated metadata, such as units.

    See :ref:`data-types`.

    .. note:: Parameters and commands can be distinguished by the ``datainfo``;
              the latter have a datainfo of ``{"type": "command", ...}``.


Optional Accessible Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"group"``
    A string identifier for grouping accessibles in the ECS.  It may contain ":"
    which may be interpreted as path separator between path components.  The
    lowercase version of a path component must not match the lowercase version
    of any module name or accessible on the same SEC node.

    Related issue: :issue:`008 Groups and Hierarchy`

    .. note:: The accessible property ``group`` is used for grouping of
              accessibles within a module, the module property ``group`` is used
              for grouping of modules within a node.

``"visibility"``
    A string indicating a hint for a GUI about the visibility of the accessible.
    Values and meaning are interpreted as for the module visibility above.

    .. note:: Setting an accessible's visibility equal or higher than its
              module's visibility has the same effect as omitting the
              visibility.  For example, a client respecting visibility in 'user'
              mode, will not show modules with 'advanced' visibility, and
              therefore also not their accessibles.


Optional Parameter Properties
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``"constant"``
    Optional, contains the constant value of a constant parameter.  If given,
    the parameter is constant and has the given value.  Such a parameter can
    neither be read nor written, and it will **not** be transferred after the
    activate command.

    The value given here must conform to the data type of the accessible.


Custom Properties
-----------------

Custom properties may further augment accessibles, modules or the SEC node
description.

As for all custom extensions, their names must be prefixed with an underscore.
The meaning of custom properties depends on the implementor, given by the
`implementor`_ module property.  An ECS that doesn't know the meaning of a
custom property MUST ignore it.  The data type of a custom property is not
pre-defined, an ECS should be prepared to handle anything here.
