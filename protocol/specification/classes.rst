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


Acqusition base classes
~~~~~~~~~~~~~~~~~~~~~~~

These base classes are meant to support devices used for data acquisition.  This
hardware can range in complexity from devices that simply provide values after
some acquisition time, to multi-channel detector setups common in large scale
facilities.

This is conceptually different simple `Readable`\s, where reading a value is
assumed to be always available, without noticeably waiting time.  In contrast,
an acqusition cycle represented by these modules can take a while to complete,
maybe providing intermediate values, and deliver a final value once finished,
until the next acquisition cycle is started.

The following terms are relevant here:

- "acquisition": a complete data acquisition system consisting of one
  AcquisitionController module and optionally some AcquisitionChannel modules.

- "acquisition cycle": a single cycle of data acquisition, started by the
  controller's `go` command.

- "channel": a part of the data acquisition system representing a measured
  quantity.  An acquisition can acquire data for one or more channels at the
  same time.

See also :ref:`rfc-106`.

.. baseclass:: AcquisitionController

    A module that controls a data acquisition process that consists of one or
    more channels. It must have a `status` that is either ``IDLE``, ``BUSY``
    while the acquisition is acquiring, or ``PREPARED`` state after ``prepare``
    is called or data acquisition is paused by `hold`.

    Descriptive data `~mod.interface_classes`: ``["AcquisitionController"]``.

    It must have a `go` command to start the data acquisition.  It is a no-op if
    already busy.  Data acquisition runs until one of the channels' active
    presets is hit or `stop` is called explicitly.  Runs the `prepare` sequence
    first if module is not already prepared.

    Optional commands:

    - `prepare`: prepares a data acquisition so that triggering with `go` is
      immediate.  No-op if already prepared.  Cannot be called when busy.
    - `hold`: pauses a data acquisition.  No-op if not busy.  Subsequent
      `go` continues the acquisition without clearing currently acquired data.
    - `stop`: stops a data acquisition.  No-op if not busy.  Subsequent `go`
      starts a new acquisition with clearing currently acquired data.

    An acquisition controller furthermore must have a `~mod.acquisition_channels`
    property specifying the channel modules belonging to this controller.  The
    names of the channel modules are represented as the values of the JSON
    object.  The role of the channels are represented by the keys and can be
    used as such by an ECS.

    The key "t" is predefined as a time channel, which basically ends
    acquisition when the time indicated by the `goal` parameter of the channel
    module is reached.

    Example module property of a controller module "controller"::

        {"t": "timechannel", "monitor": "monitor_channel"}

    The 3 modules "controller", "timechannel" and "monitor_channel" all belong
    together.

.. baseclass:: AcquisitionChannel

    A module that represents one component of a data acqusition.  Multiple
    channels can be combined within a controller.

    Descriptive data `~mod.interface_classes`: ``["AcquisitionChannel",
    "Readable"]``.

    It must have a `value` parameter which is interpreted as in `Readable`.  If
    the hardware allows, this parameter should be readable during the
    acquisition and return the current intermediate state.  Outside of a data
    acquisition, the value MUST stay unchanged and represent the result of the
    latest cycle.

    It also must have a standard `status` parameter that is ``BUSY`` while the
    channel is acquiring.

    Optionally, it can have a `goal` parameter.  It is a `value` that, when
    reached, stops the data acquisition.  Depending on the nature of the
    acqusition cycle being performed, it may or may not be useful to configure
    the acqusition with a `goal`.  It can for example represent time for timer
    channels, or a certain number of events for event counter channels, or a
    desired statistical significance for a channel that represents the
    measurement uncertainty.  For acquisitions that are configured with several
    parameters whose value is unrelated to the main `value` parameter, it is
    better to use custom parameters instead.

    Furthermore, it can have a ``goal_enable`` parameter.  If false, the goal is
    ignored and the acquisition does not stop due to this channel.  If `goal` is
    present but not ``goal_enable``, the goal is always active.

    **Optional extension for channels that have "matrix" like values**

    - `roi`, an optional parameter: a list containing a ``(min, max)`` tuple
      per dimension, to specify a sub-slice of matrix data to consider in
      `value` and return in `get_data`.
    - `get_data`, an optional command: returns the channel's data, with a
      :ref:`matrix <matrix>` data type.

    The `value` parameter only contains a useful "reduced" form of the data,
    for example, the sum of all events in the matrix, or the average of all
    intensity values in a spectrum.  If `roi` exists, only the selected
    sub-slice of matrix data is considered for this reduction.

.. baseclass:: Acquisition

    Combines both AcquisitionController and AcquisitionChannel accessibles into
    one interface, for simple devices where only one channel is needed.

    Does not have the `~mod.acquisition_channels` property.

    Descriptive data `~mod.interface_classes`: ``["Acquisition", "Readable"]``.


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
