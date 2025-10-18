- Feature: Acquisition interface classes
- Status: Open
- Submit Date: 2025-02-12
- Authors: Georg Brandl <g.brandl@fz-juelich.de>
- Type: Modules
- PR:

Summary
=======

This RFC (carried over from `issue 76
<https://github.com/SampleEnvironment/SECoP/blob/master/issues/076%20Interface%20for%20Measurable%20hardware.rst>`_)
proposes the addition of two new interface classes, specific to modules that
support longer-running data acquisition.


Goal
====

In order to support a wider spectrum of hardware, SECoP should support
integrating devices used for data acquisition, and define appropriate
interfaces.  This hardware can range in complexity from devices that simply
provide values after some acquisition time, to multi-channel detector setups
common in large scale facilities.

This is conceptually different from a simple ``Readable``, where reading a
value is assumed to be always available, without noticeably waiting time.
In contrast, an acqusition cycle represented by these modules can take a
while to complete, maybe providing intermediate values, and deliver a final
value once finished, until the next acquisition cycle is started.


Technical explanation
=====================

It is proposed to add three new interface classes: ``AcquisitionController``,
``AcquisitionChannel`` and ``Acqusition``.


Definitions
-----------

- "acquisition": a complete data acquisition system consisting of one
  AcquisitionController module and optionally some AcquisitionChannel modules.

- "acquisition cycle": a single cycle of data acquisition, started by the
  controller's ``go`` command.

- "channel": a part of the data acquisition system representing a measured
  quantity.  An acquisition can acquire data for one or more channels at the
  same time.

- "matrix type channel": a channel whose measured data is not a single
  value/handful of values that is manageable as the ``value`` parameter.


``AcquisitionController`` (no base interface)
---------------------------------------------

Descriptive data ``interface_classes``: ``["AcquisitionController"]``.

Accessibles:

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the acquisition is acquiring.
    The module is in ``PREPARED`` state after ``prepare`` is called or data
    acquisition is paused by ``hold``.

command ``prepare``
    Optional command: prepares a data acquisition so that triggering with ``go``
    is immediate.  No-op if already prepared.  Cannot be called when busy.

command ``go``
    Mandatory command: starts a data acquisition.  No-op if busy.
    Data acquisition runs until one of the channels' active presets is hit or
    ``stop`` is called explicitly.  Runs the ``prepare`` sequence first if
    module is not already prepared.

command ``hold``
    Optional command: pauses a data acquisition.  No-op if not busy.
    Subsequent ``go`` continues the acquisition without clearing currently
    acquired data.

command ``stop``
    Optional command: stops a data acquisition.  No-op if not busy.
    Subsequent ``go`` starts a new acquisition with clearing currently
    acquired data.

Property:

``acquisition_channels``
    A JSON object specifying the channel modules belonging to this AcquisitionController.
    The names of the channel modules are represented as the values of the JSON object.
    The role of the channels are represented by the keys and can be used as such by
    an ECS.

    The key "t" is predefined as a time channel, which basically ends acquisition when
    the time indicated by the ``goal`` parameter of the channel module is reached.

    Example module property of a controller module "controller"::

        {"t": "timechannel", "monitor": "monitor_channel"}

    The 3 modules "controller", "timechannel" and "monitor_channel" all belong together.

    This property is mandatory on any ``AcquisitionController``.


``AcquisitionChannel`` (derived from ``Readable``)
--------------------------------------------------

Descriptive data ``interface_classes``: ``["AcquisitionChannel", "Readable"]``.

Accessibles:

``value``
    Mandatory: Interpretation as in ``Readable``.

    If the hardware allows, this parameter should be readable during the
    acquisition and return the current intermediate state.

    Outside of a data acquisition, the value MUST stay unchanged and
    represent the result of the latest cycle.

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the channel is acquiring.

``goal``
    Optional: a ``value`` that, when reached, stops the data acquisition.

    Depending on the nature of the acqusition cycle being performed, it
    may or may not be useful to configure the acqusition with a ``goal``.
    It can for example represent time for timer channels, or a certain
    number of events for event counter channels, or a desired statistical
    significance for a channel that represents the measurement uncertainty.

    For acquisitions that are configured with several parameters whose value
    is unrelated to the main ``value`` parameter, it is better to use custom
    parameters instead.

``use_goal``
    Optional: a Boolean, if false, the goal is ignored and the acquisition
    does not stop due to this channel.

    If ``goal`` is present but not ``use_goal``, it is never ignored.


"Matrix" type channels
~~~~~~~~~~~~~~~~~~~~~~

Not an additional interface class, but an optional extension of
``AcquisitionChannel``.

Accessibles:

``roi``
    Optional: a list containing a ``(min, max)`` tuple per dimension, to specify
    a sub-slice of matrix data to consider in ``value`` and return in
    ``get_data``.

command ``get_data``
    Optional: returns the channel's data, with a ``matrix`` data type.

The ``value`` parameter only contains a useful "reduced" form of the data, for
example, the sum of all events in the matrix, or the average of all intensity
values in a spectrum.  If ``roi`` exists, only the selected sub-slice of matrix
data is considered for this reduction.

``binning``
    Optional: allows reduction of the matrix size by re-binning data already
    on the server side.  (Precise semantics to be specified.)

``axes``
    Optional: a list of axes ticks for the dimensions of the matrix data, if
    useful (i.e. not just "pixel 1..N").  (Precise semantics to be specified.)


``Acquisition`` (derived from ``Readable``)
-------------------------------------------

Combines both AcquisitionController and AcquisitionChannel accessibles into one
interface, for simple devices where only one channel is needed.

Does not have the ``acquisition_channels`` property.

Descriptive data ``interface_classes``: ``["Acquisition", "Readable"]``.


Disadvantages, Alternatives
===========================

Disadvantages
-------------

None except for more complexity in the specification.

Alternatives
------------

- Instead of having three new classes, let ``AcqusitionController`` optionally
  have the interface of ``AcquisitionChannel`` as well.  However, this gets
  messy and repetitive when later more accessibles for the channel class are
  added.


Formerly Open Questions
=======================

- Should we add an optional parameter ``progress`` on the
  ``AcqusitionController``, which gives an (approximate) percentage (or
  elapsed/remaining timings) for the acquisition process?

  (This has been deferred to when a use-case is brought up and may be added
  as a generic SECoP feature.)
