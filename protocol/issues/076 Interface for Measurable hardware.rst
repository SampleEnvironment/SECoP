SECoP Issue 76: Interface for Measurable hardware (proposed)
============================================================

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


``MeasurableController`` (no base interface)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Accessibles:

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the measurable is acquiring.
    The module is in ``PREPARED`` state after ``prepare()`` is called or data
    acquisition is paused by ``hold()``.

``prepare()``
    Optional command: prepares a data acquisition so that triggering with ``go``
    is immediate.  No-op if already prepared.  Cannot be called when busy.

``go()``
    Mandatory command: starts a data acquisition.  No-op if busy.
    Data acquisition runs until one of the channels' active presets is hit or
    ``stop`` is called explicitly.  Runs the ``prepare()`` sequence first if
    module is not already prepared.

``hold()``
    Optional command: pauses a data acquisition.  No-op if not busy.
    Subsequent ``go()`` continues the acquisition without clearing currently
    acquired data.

``stop()``
    Optional command: stops a data acquisition.  No-op if not busy.
    Subsequent ``go()`` starts a new acquisition with clearing currently
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
    ``get_data()``.

``get_data()``
    Optional: returns the channel's matrix data, see below for details.

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


The return value of ``get_data()``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since the returned data matrix can get pretty large, it is not efficient to
encode it as nested ``Array``\s.  Instead, the data is kept in Blob form
(base64-encoded for JSON transmission).

The return value is a struct, where the following members are currently
specified:

- ``data``: The data as a ``Blob``, whose datainfo must have these additional
  properties:

  - ``elementtype``: The type (and size, and byte order) of each matrix element,
    a string in Numpy convention (e.g. ``"<u4"``).
  - ``names``: A list of names for each dimension
  - ``maxlengths``: A list of maximum lengths for each dimension

- ``dims``: An array containing the actual lengths of each dimension.

The order of the matrix elements is defined so that the dimension with the
fastest running index comes first in ``dims``, ``names`` and ``maxlengths``.

Example: ``data`` is ``[1, 2, 3, 4, 5, 6]``, ``dims`` is ``[2, 3]`` and
``names`` is ``["x", "y"]``.  Then the matrix looks as follows::

  .     x=0 x=1
  y=0   1   2
  y=1   3   4
  y=2   5   6


Remarks
~~~~~~~

- All modules belonging to one measurable SHOULD have a ``group`` property,
  which is set to the same identifier.


Discussion
----------

