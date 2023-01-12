SECoP Issue 78: Interacting Modules - use case power supply (proposed)
======================================================================

Motivation
----------

For the use case of a power supply with two or more modules
representing voltage, current and even power or resistance
it would be desirable to standardize how to implement
and make visible the interlink between the modules.


Proposal
--------

Interlinked modules as for a power supply should be implemented in the following way:

* the interlinked modules SHOULD have the a 'group' property set to the same
  identifier
* each of the modules must have a 'quantity' string property describing the
  physical quantity. example values: 'current', 'voltage', 'power', 'resistance'
* such modules might have one or more "max_<quantity>" parameters representing
  a limit, the hardware tries to respect. <quantity> is one of the other
  quantities the hardware supports, but is distinct from 'quantity' property
  of the module itself.
* when more than one of the modules is a Writable, these modules must have
  a 'control_active' parameter indicating which module is actively controlling
* certainly, this mechanism could be used also for other physical quantities
  than power supplies


Discussion
----------

* this proposal interferes with `SECoP Issue 77: predefined parameter name prefixes`_.
  It might be desirable to choose different prefixes for max_<quantity> and max_<parameter>
* is it necessary to add a list of standardized quantity names into the standard?

.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 77: predefined parameter name prefixes`: 077%20predefined%20parameter%20name%20prefixes.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
