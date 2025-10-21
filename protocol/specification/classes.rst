.. _interface-classes:

Interface classes
=================

Interface classes let the ECS determine the functionality of a module from its
class or classes.

The standard contains a list of classes, and a specification of the
functionality for each of them.  The list might be extended over time.  Already
specified base classes may be extended in later releases of the specification,
but earlier definitions will stay intact, i.e. no removals or redefinitions will
occur.

The module class is in fact a list of classes (highest level class first) and is
stored in the module-property `interface_classes <mod.interface_classes>`.  The
ECS chooses the first class from the list which is known to it.  The last one in
the list must be one of the base classes listed below.

.. admonition:: Remark

    The list may also be empty, indicating that the module in question does not
    even conform to the Readable class.


Base classes
------------

.. baseclass:: Communicator

    The main purpose of the module is communication.  It may have none of the
    predefined parameters of the other classes.

    The `communicate` command should be used mainly for debugging reasons, or
    as a workaround for using hardware functionalities not implemented in the
    SEC node.

.. baseclass:: Readable

    The main purpose is to represent readable values (i.e. from a Sensor).
    It has at least a `value` and a `status` parameter.

.. baseclass:: Writable

    The main purpose is to represent fast settable values (i.e. a switch).
    It must have a `target` parameter in addition to what a `Readable` has.
    It does not have a `stop` command. A module which needs time to reach
    the target but cannot be stopped should also be represented as a `Writable`,
    with a ``BUSY`` item (code 300...389) in the status enum.

.. baseclass:: Drivable

    The main purpose is to represent slow settable values (i.e. a temperature or
    a motorized needle valve).  It must have a `stop` command in addition to
    what a `Writable` has.  Note that in case the `stop` command has no
    effect, a `Writable` SHOULD be used.  Also, the `status` parameter will
    indicate a ``BUSY`` state for a longer lasting operations.


.. _features:

Features
--------

Features allow the ECS to detect if a SECoP module supports a certain
functionality.  A feature typically needs some predefined parameters, commands
and/or module properties to be present.  However, it is not only a list of
mandatory or optional elements, but also indicates to the ECS that it may handle
this functionality in a specific way.

.. feature:: HasOffset

    This feature indicates that the `value` and `target` parameters of a
    module represent raw values, which need to be corrected by an offset.  A
    module with the feature `HasOffset` must have a parameter `offset`,
    which indicates to all clients that the transmitted raw values for the
    parameters `value` and `target` are to be converted to corrected values
    (on the client side) by the following formulas:

    For reading the parameters `value` and `target`:

    | corrected value (client) = value (transmitted) + offset
    | corrected target (client) = target (transmitted) + offset

    For changing the parameter `target`:

    | target (transmitted) = corrected target (client) - offset

    Mandatory parameter: `offset`
