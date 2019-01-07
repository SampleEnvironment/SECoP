SECoP Issue 36: Dynamic units
=============================

Motivation
-----------
In the current specification the unit of the main value of a module is fixed by the properties.
While this is a well-working approach for most situations, sometimes a 'changeable' unit is needed.

This essential has 2 use cases:
  1) a SEC-node may whish to indicate a different unit
     (unit was changed directly on the front-panel on e.g. a lakeshore,
     or some other measuring device (keithley, leybold, ...))

  2) an ECS may want to change the unit of an module
     (i.e. switch between °C and K on an oven or between mK and K on a :sup:`3`\ He
     or :sup:`3`\ He/:sup:`4`\ He dilution cryostat)

Both uses case require slightly different adoptions of the protocol.
A solution to use case 2) should include the solution to use case 1),
so that the SEC-Node can reflect the changed unit back towards the ECS.
This is especially important as a unit change of the main value may need to be reflected on other parameters as well.
(i.e. a `ramp` may also need to be adjusted, both in value and in unit.)

:related: Issue 43

Proposal
--------
For use case 1) it would be sufficient to add an "u" qualifier containing the current unit as a string.
Of course this SHOULD only be sent, if the unit differs from the one given in the structure report.

.. note:: all parameters, whose unit is influenced by this also have to reflect the changed unit.

To solve use case 2), an additional parameter "unit" should be defined as Enum, listing all the
units supported by the SEC-node for the main value of that module.

.. note:: Implementors of an SEC-node supporting this MUST be aware, that all side-effects need to be realized and communicated
          before the reply to the change request is sent, resulting in at least one update message before the reply (if updates are enabled).
          This is also true, if the unit gets re-set to its current value!
          (in that case, only updates of all parameters must be sent, whose unit would have changed).

          :related: Issue 31

The unit sent in the qualifier, however, MUST always be the unit-string, even though the value
of the unit parameter gets transferred as a number (as it's an Enum).


Discussion
----------
topic brought up once but was not so clearly specified and discussion was postponed until a use
case would be found. Use case was found now and given in the Motivation.

No discussion of this issue in its current form yet.

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Agreement about the proposal, it should just not be included now.
Also, it should be an extension and will not belong to the core.

Issue is to be kept open for discussion for later inclusion.

video conference 2018-12-03
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- for now this issue is to be closed. To be reopened only if new use cases arised, which can not be solved nicely by other means.
