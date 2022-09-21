Draft of a Cryomagnet
=====================

:authors:
    Markus Zolliker

:Version: 0.0 of 2022-09-21


Introduction
------------

A cryomagnet may be seen as a helium cryostat combined with a superconducting coil.
Therefore a template for a cryomagnet would be composed of template for a superconducting
coil power supply and from the known (orange like) liquid helium cryostat.

Superconducting Coil Power Supply
---------------------------------

The typical superconducting power supply has a switch heater and a rampable current output.

We may split into 3 drivable modules:

main module
~~~~~~~~~~~

- parameters ``value`` and ``target``, double with unit='Tesla'
- parameter ``status``, with up to 18 status codes as defined in SECoP
- parameter ``mode`` with enum values ``DISABLED``, ``DRIVEN``, and ``PERSISTENT``
- parameter ``ramp``, double with unit = ``Tesla/min``
- command ``stop`

possibly some parameters defining the specifics of like waiting times
for stablizing current.

switch heater
~~~~~~~~~~~~~

- parameter ``value`` and ``target`` either bool or enum (OFF=0, ON=1)
- parameter ``status`` (standard enum for a drivable)
- command ``stop`` (doing nothing!)

possibly some parameters defining the specifics of like waiting times
for switching on and off.


current output
~~~~~~~~~~~~~~

- parameters ``value`` and ``target``, double with unit='A'
- parameter ``status``, standard set
- parameter ``ramp``, double with unit = ``A/min``
- command ``stop`


Discussion:
-----------

We may as well integrate the latter 2 modules as parameters into the main module.
In contrast to the orange example, we do not want to use the switch heater and
current output on its own, may be with the exception of debugging purposes.
So it is really debateable if it the splitting in to 3 modules does not make
it to complicated.

In the existing examples at PSI we use just one module, and we use 'Tesla'
as units for the leads current as units.
