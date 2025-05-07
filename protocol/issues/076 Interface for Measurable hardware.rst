SECoP Issue 76: Interface for Measurable hardware (closed)
==========================================================

Note
----

Discussion has moved to SECoP RFC 006.


Motivation
----------

In order to support a wider spectrum of hardware, SECoP should support
integrating devices used for data acquisition, and define appropriate
interfaces.  This hardware can range in complexity from devices that simply
provide values after some acquisition time, to multi-channel detector setups
common in large scale facilities.


Definitions
-----------

- "Measurable": a complete data acquisition system consisting of one controller
  module and one or more channel modules.

- "Data acquisition": a single cycle of data acquisition, started by the
  controller's ``go`` command.

- "Channel": a part of the data acquisition system representing a measured
  quantity.  A measurable can acquire data for one or more channels at the same
  time.

- "Matrix type channel": a channel whose measured data is not a single
  value/handful of values that is manageable as the ``value`` parameter.


Proposal
--------

It is proposed to add three new interface classes.


``Controller`` (no base interface)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Accessibles:

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the measurable is acquiring.
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


``MeasurableChannel`` (derived from ``Readable``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Accessibles:

``value``
    Mandatory: Interpretation as in ``Readable``.

    If the hardware allows, this parameter should be readable during the
    acquisition and return the current intermediate state.

    Outside of a data acquisition, the value MUST stay unchanged and
    represent the result of the latest cycle.

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the measurable is acquiring.

``preset``
    Optional: a value that, when reached, stops the data acquisition.
    In most cases, this will exist, and at least one channel MUST have
    a preset.

    Interpretation is channel specific: It can represent time for timer
    channels, or a certain number of events, or even a desired statistical
    significance.

``use_preset``
    Optional (but must be there when ``preset`` is): a Boolean, if false, the
    preset is ignored and the acquisition does not stop due to this channel.


``Measurable``
~~~~~~~~~~~~~~

Combines both MeasurableController and MeasurableChannel accessibles into one
interface, for simple devices where only one channel is ever needed.


"Matrix" type channels
~~~~~~~~~~~~~~~~~~~~~~

Not an additional interface class, but an optional extension of
``MeasurableChannel``.

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


Remarks
~~~~~~~

- All modules belonging to one measurable SHOULD have a ``group`` property,
  which is set to the same identifier.


Discussion
----------

