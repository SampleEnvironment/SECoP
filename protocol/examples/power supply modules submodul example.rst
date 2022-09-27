Module granularity and simple example for submoduling
#####################################################

:authors:
	Anders Pettersson;
	Niklas Ekström

For us it feels more natural to first define simple lowest level modules, then can be used in higher level modules like cryostat modules.

In the following examples we would like to aim the discussion rater at what level of granularity we can have on our templates and modules than the functionality of the examples by themselves.
It also brings up issue 65, and gives a simple example, easier for us to grasp.
This should be locked at as building two different power supplies from the same (sub)modules (the submodules are described last). The different types are also examples on when one module controls two submodules in "parallel" and in "layers"



Constant V/ Constant I Power Supply module
==========================================

Introduction
------------
A power supply depends on what modules it controls.
It has always at least one power source to choose from.
Normally a constant voltage source or a constant current source or both as in this case.
We use general power source templates to describe these.
See: constant voltage source and Draft of constant current source templates below.


CV/CI Power Supply module
-------------------------

- parameters ``value`` and ``target``, struct(more than one parallel modules) with members from sub modules. {"voltage": <double>, "current": <double>} The units are also inherited from submodule(s).
- parameter ``status``, inherits statuses from sub templates, and adds new statuses from the 18 status codes as defined in SECoP..
- parameter ``sub_modules`` tuple [constant voltage source, constant current source]
- other parameters from submodules...(thinking frame/tab in GUI here)

Discussion:
-----------

Cryomagnet Power Supply module
==============================

Introduction
------------
A cryomagnet power supply has a ramp functionality. The ramp functionality is described in a general "ramp_module",
the ramp module controls a general constant current source.


Cryomagnet Power Supply module
------------------------------

- parameters ``value`` and ``target``, from sub module. The units are also inherited from submodule(s).
- parameter ``status``, inherits statuses from sub templates, and adds new statuses from the 18 status codes as defined in SECoP.
- parameter ``controlled_by`` (if it´s meant to be governed by another module)
- parameter ``control_active``
- parameter ``sub_modules`` tuple [Ramp module[constant voltage source]]
- other parameters from submodules...

Discussion:
-----------


Constant voltage source
=======================

Introduction
------------

A constant voltage source delivers constant voltage independent of the current drawn. Current should be within limits.

Constant voltage Power Supply
-----------------------------

- parameters ``value`` and ``target``, double with unit='Volt'
- parameter ``status``, with up to 18 status codes as defined in SECoP
- parameter ``current limits``, min and max current where it can deliver set voltage.
- parameter ``controlled_by``
- parameter ``control_active``




Discussion:
-----------


Constant current source
=======================

Introduction
------------

A constant current source delivers constant current independent of the voltage set. Voltage should be within limits.

Constant current Power Supply
-----------------------------

- parameters ``value`` and ``target``, double with unit='Volt'
- parameter ``status``, with up to 18 status codes as defined in SECoP
- parameter ``voltage limits``, min and max voltage where it can deliver set current.
- parameter ``controlled_by``
- parameter ``control_active``

Discussion:
-----------

Ramp module
===========

Introduction
------------

A ramp module ramps output to a given set point under a given time or rate.

Ramp module
-----------
- parameters ``value`` and ``target``, struct with members from sub modules. The units are also inherited from submodule(s).
- parameter ``time_unit``
- parameter ``status``, inherits statuses from sub templates, and adds new statuses from the 18 status codes as defined in SECoP.
- parameter ``controlled_by``
- parameter ``control_active``
- parameter ``ramp rate``, speed of the ramp.
- parameter ``time``, time to reach set point.
- parameter ``sub_modules`` tuple e.g. constant voltage source, constant current source, valve_actuator, etc.
- other parameters from submodules...

Discussion:
-----------


