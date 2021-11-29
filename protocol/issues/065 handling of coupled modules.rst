SECoP Issue 65: handling of coupled (sub)modules (proposed)
===========================================================

Motivation
----------

Sometimes Modules 'interact' with the same physical 'output'.
Currently it is undefined on how to implement this, yet
having a defined standard way would greatly improve interoperability.

Several ideas exist, ranging from: 'put everything in one module and
add parameter selecting the main role' to 'make everything it's own module'.
In the latter case we would need:

1. a way to 'mark' the corresponding' modules (or group of modules?)
2. a defined way to 'switch over' the module in charge
3. a way to indicate, which module is 'in charge'

The only thing clear so far is that the content and meaning of the 'value' parameter should not be changed at runtime,
i.e. if it is defined to repesent a temperature, it will never 'be switched' to resistance or heater_power or anything else.


Proposal
--------

First the ECS needs to know, which modules and/or parameters are relevant to
each other. This could be done with an additional property ``"influences"`` to
these modules and parameters, which contains the other module and/or parameter
names as *array of strings*. If we decide, that the influenced parameters
have to be inside the *same* module, the module name could be omitted.

A predefined list of prefixes or suffixes are not meaningful here, because this
list will almost never match the needs of a SEC node implementor.

1) *use enable / disable technique*

   This works best for separated modules. This addresses a similar problem like
   `SECoP Issue 22: Enable Module instead of Shutdown Command`_ , but here we
   what to switch over to the module in charge (and not initialize/shutdown).

   The ECS knows the influenced modules and is prepared, that switching of a
   module in charge means, that the previous module is no longer in charge and
   may change its status.

   a) We could extend the ``"enable"`` parameter (was a flag only).

      The ECS could easily switch to the wanted module or see, what module is
      currently in charge.

   b) Introduce a new flag ``"module_in_charge"`` (aside from ``"enable"``).

    A change command to the proposed parameter, could deny a change to disable
    the module in charge or deny to enable the module in charge, because of
    it's internal state.

    We should switch the module in charge by simply specifying is with a
    ``"change"`` command. A dedicated ``"do"`` command has no advantage here.

2) *mode / status technique*

   This could be done inside a module or between separate modules.
   But it conflicts with `SECoP Issue 48: mode parameter`_ and
   `SECoP Issue 59: set_mode and mode instead of some commands`_ , because a
   ``"mode"`` parameter could also mean anything else. Something like a
   "status"-parameter could show, which module or parameter is in charge.

   This is the very complex and leads to incompatible implementations,
   which should be avoided.

Discussion
----------

Decision
--------


.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 22: Enable Module instead of Shutdown Command`: 022%20Enable%20Module%20instead%20of%20Shutdown%20Command.rst
.. _`SECoP Issue 48: mode parameter`: 048%20mode%20parameter.rst
.. _`SECoP Issue 59: set_mode and mode instead of some commands`: 059%20set_mode%20and%20mode%20instead%20of%20some%20commands.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
