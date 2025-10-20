Structure of SECoP
==================

.. image:: images/secop-img/secop-node-instance.svg

.. index:: ECS
.. index:: SEC node

SECoP is a communication protocol with a client-server model.  The server is
called the **SEC node**, while the client in here is called the **ECS**.

ECS
    Software controlling the hardware for carrying out an experiment.  Includes the
    user interface.  Usually speaks several protocols with different parts of the
    instrument.  Often also called "instrument control" in short.

SEC node
    A computing unit or process, connected to all control units (temperature
    controller, flow controller, pressure sensor ...) of a sample environment,
    bridge to the ECS.  SECoP specifies how ECS speak with a SEC node.  The SEC
    node allows ECS to access a set of modules (and their parameters/commands)
    via SECoP.  It also provides a list of accessible modules and parameters as
    well as descriptive meta data.

.. index:: module

Each SEC node consists of one or more **Modules**.  These represent the main
interface for interacting with the hardware.

Modules usually have one or more **Base classes** assigned that indicate their
basic functionality, split into **Interface classes** and **Features**.

Module
    A named logical component of an abstract view of the equipment.

    We intentionally avoid the term "device", which might be misleading, as
    "device" is often used for an entire apparatus, like a cryomagnet or
    humidity cell.  In the context of SECoP, an apparatus in general is composed
    of several modules.  For example different temperature sensors in one
    apparatus are to be seen as different modules.

    Most modules should correspond to one independently measurable physical
    quantity and use one of the interface classes `Readable`, `Writable` or
    `Drivable`.  However, more specialized modules like `Communicator` can be
    implemented where appropriate.

    A SEC node controls a set of named modules.  Modules are fully specified by
    the descriptive data, see :ref:`module-description`.

.. index:: interface class
.. index:: feature

Interface class
    A named grouping of parameters, commands and properties on a module that
    specifies a certain capability.

    Adherence to interface classes (such as `Drivable`) greatly simplifies
    implementation in the ECS, since the basic functionality of any Drivable can
    be handled in exactly the same way.

Feature
    Similar to an interface class, a feature specifies a elements and
    functionality that are supported by a module, however it is not bound to a
    specific hierarchy and can be added to any module regardless of interface
    class.

.. index:: parameter
.. index:: command
.. index:: accessible

Each of these modules can have static values which are known at startup and
dynamic values.  These are called **Properties** and **Parameters**
respectively.  Parameters can in turn have their own Properties.  Examples of
properties would be the datatype of the parameter or the `readonly` flag that
shows whether a parameter may be written to or not.

To initiate actions that may not necessarily be tied to a parameter, modules can
also have **Commands**, like stopping the current movement or running a
calibration.

Accessible
    Parameters and commands together are called Accessibles.

Parameter
    A piece of data associated with a module, typically in one of three
    categories:

    - physical or hardware defined, e.g. `value` or ``pid``
    - informational, e.g. `status`
    - controlling the operation of the module, e.g. `pollinterval`

    The main parameter of a module is its `value`.  Writable parameters may
    influence the measurement (like PIDs).  Additional parameters may give more
    information about its state (running, target reached), or details about its
    functioning (heater power) for diagnostics purposes.

    Parameters with a predefined meaning are listed in :doc:`the standard
    <accessibles>`, they must always be used in the same way.  Custom parameters
    are defined by the implementation of the SEC node and their name must start
    with an underscore.  The ECS can use unknown parameters only in a generic
    way, as their meaning is not known.

    Parameters are fully specified by the descriptive data, see
    :ref:`accessible-description`.

Command
    Commands are provided to initiate specified actions of the module.
    They should generate an appropriate reply immediately after that action is
    initiated, i.e. should not wait until some other state is reached.

    However, if the command triggers side-effects, they MUST be communicated
    before the reply is sent.  Commands may use a possibly structured argument
    and may return a possibly structured result.

    Commands with a predefined meaning are listed in :doc:`the standard
    <accessibles>`, they must always be used in the same way. Custom commands
    are defined by the implementation of the SEC node, the ECS can use them only
    in a general way, as their meaning is not known.

.. index:: property
.. index:: data info

Property
    The static information about SEC nodes, modules, parameters/commands and
    their data types is constructed from properties with predefined names and
    meanings.  They constitute the "self-describing" part of SECoP.

    All properties are collected in the so-called "structure report" and sent to
    clients on request, as described in :ref:`this section <descriptive-data>`.

Data info
    The full information about data type and metadata of the value of all
    parameters, and argument/return value of all commands, is called "data info"
    specified in the descriptive data.  Available data info is specified in
    :doc:`this section <datainfo>`.
