SECoP Issue 48: mode parameter
==============================

Motivation
----------

Define mechanism to select one of several operation modes of a module.

Proposal
--------

pre-define the parameter ``mode`` as an enum, listing the defined operation modes.
Since the available operation modes depend on the specific module, they can not be predefined in the specification.

An example might help here:

a temperate controller module may define the mode as follows:

.. code::

   ["enum",{"openloop":0,"pid":1,"ramp":2}]

i.e. it supports three modes: "openloop", "pid" and "ramp".
What they actually do should be described in the description, the ``mode`` parameter is intended
to provide a defined way to switch operation modes.


Discussion
----------

not discussed in its present form.

vidconf 2018-12-03
~~~~~~~~~~~~~~~~~~

- decision to reserve the parameter name 'mode' for this purpose
