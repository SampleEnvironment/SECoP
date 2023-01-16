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

It is proposed to add two new interface classes.

:note: All names are provisional subject to finding better ones.


``MeasurableController`` (no base interface)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Accessibles:

``status``
    Mandatory: Standard SECoP status.
    The module is in ``BUSY`` state while the measurable is acquiring.

``go()``
    Mandatory command: starts a data acquisition.  No-op if running.
    Data acquisition runs until one of the channels' active presets is hit or
    ``stop`` is called explicitly.

``stop()``
    Mandatory command: stops a data acquisition.  No-op if not running.

``clear()``
    Mandatory command: clears/resets accumulated data on all channels.
    If the hardware allows, this can be called during an acquisition.


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

``clear()``
    Mandatory command: clears/resets accumulated data on this channel.
    If the hardware allows, this can be called during an acquisition.

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

Since the returned data matrix can get pretty large, it is not clear how to best
marshal it in the SECoP protocol, and correspondingly which data-type to use.
Candidates are:

- Use nested ``Array``\s.

  Disadvantages: For >=2 dimensions, subarrays could be of inconsistent
  lengths, and the JSON marshaling is very inefficient, since the data
  is usually available in contiguous form in memory (e.g. Numpy arrays)
  and is usually wanted in the same form again on the client side.

  Furthermore, no name is possible to define for the dimensions unless the
  Array data info is amended with such an entry.

  Advantages: The size per dimension is self-describing.

- Create a separate new data info type, which defines the N dimensions of a
  matrix with name, maximum size and type of each element.  The data can be
  transferred directly from contiguous memory in binary form (base64-encoded for
  JSON transmission).

  Disadvantages: The size per dimension must be somehow added to the data, since
  it can change.

  Advantages: Data can be further transformed (e.g. using compression).

- The same, but use ``Blob`` and define additional data properties.


Remarks
~~~~~~~

- In the case of simple measurables, which consist of a single channel only, the
  two interfaces can be implemented in a single module.

- All modules belonging to one measurable SHOULD have a ``group`` property,
  which is set to the same identifier.


Discussion
----------

