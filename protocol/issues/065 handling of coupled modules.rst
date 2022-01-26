SECoP Issue 65: handling of coupled (sub)modules (finalizing)
=============================================================

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
Having an optional, read-only parameter ``controlled_by`` (on Drivables/Writables)
solves the issue, if all participants are defined within one secnode.
On distributed systems, proxy modules for the controlled module must be generated
on the SEC-Node of the controlling module.
If that parameter is not an empty string, the module is to be treated like a Readable
module. Otherwise it must name a module of the same sec-node.

After discussing the consequences, an enum instead of a string is preferred, with a default value
of 0:'self' meaning the module is not controlled by some other module. Other values should
'name' the potential controllers of this module.

Example by Markus
-----------------

This should explain how the mechanism should work from his point of view.

Consider the following fictive, but not unrealistic example:

Our system has 5 Modules:

T_reg, T_sample (both Drivable):
   Regulation and sample T, both with a control loop driving P_heater and p_nv
   and a boolean parameter '_auto_nv' telling if the needle valve pressure should
   be changed based on the difference of target and value of T_reg or T_sample.
   Only one of the control loops might be active.

P_heater (Writable):
   the heater power

p_nv (Drivable):
   needle valve pressure, with a control mechanism driving pos_nv

pos_nv (Drivable):
   the needle valve motor


If the parameter ``output_active`` is false, it has the meaning:
The output (= the target of the controlled module) is not changed anymore. 
For a temperature: the control loop is disabled, the heater power and the
needle valve pressure target is not touched by the temperature module.
For the pressure regulating needle valve: the control mechanism is disabled,
the needle valve motor is not moved by ``p_nv``.


.. table:: Here a table of possible situations:

    ========================= ===== ===== ===== ======== ======== ======== ===== =====
    situation                 1a    1b    1c    2a       2b       2c       3b    3c
    ========================= ===== ===== ===== ======== ======== ======== ===== =====
    T_reg:output_active       true  true  true  false    false    false    false false
    T_sample:output_active    false false false true     true     true     false false
    P_heater:output_active    true  true  true  true     true     true     true  true
    p_nv:output_active        true  true  false true     true     false    true  false
    pos_nv:output_active      true  true  true  true     true     true     true  true
    T_reg:_auto_nv            true  false false x        x        x        x     x
    T_sample:_auto_nv         x     x     x     true     false    false    x     x
    P_heater:controlled_by    T_reg T_reg T_reg T_sample T_sample T_sample self  self
    p_nv:controlled_by        T_reg self  self  T_sample self     self     self  self
    pos_nv:controlled_by      p_nv  p_nv  self  p_nv     p_nv     self     p_nv  self
    ========================= ===== ===== ===== ======== ======== ======== ===== =====

``x`` indicates: does not matter

* 1a) regulation on T_reg with automatic n.v. control (dependent on T)
* 1b) regulation on T_reg with n.v. pressure regulated on a given pressure
* 1c) regulation on T_reg with manual n.v. position
* 2abc) regulation on T_sample, else as above
* 3b) manual heater power, n.v. pressure regulated on a given pressure
* 3c) manual heater power, manual n.v. position


.. table:: The following table describes what happens when the target of a module is changed:

    ========================= ========= ========= ========= ========= =========
    target changed on         T_reg     T_sample  P_heater  p_nv      pos_nv
    ========================= ========= ========= ========= ========= =========
    P_heater:controlled_by    T_reg     T_sample  self
    p_nv:controlled_by        T_reg*    T_sample*           self
    pos_nv:controlled_by                                    p_nv      self
    T_reg:output_active       true      false     false
    T_sample:output_active    false     true      false
    P_heater:output_active    true      true      true
    p_nv:output_active        true*     true*
    pos_nv:output_active      true*     true*
    T_reg:_auto_nv                                          false     false
    T_sample:_auto_nv                                       false     false
    situation afterwards      1x        2x        3y        nb        nc
    ========================= ========= ========= ========= ========= =========

| ``x`` indicates: switch to ``a`` when _auto_nv is true, else keep ``a``, ``b`` or ``c`` as before
| ``y`` indicates: keep ``b`` or ``c`` as before
| ``n`` indicates: keep  ``1``, ``2`` or ``3`` as before
| ``*`` indicates: value is changed only when _auto_nv is true


Conclusion 1:
   As we can see, there is no situation where ``P_heater:output_active`` or
   ``pos_nv:output_active`` has to be false. Which means that this parameter is
   not really needed here.

Conclusion 2:
   If inner mechanics of the system is known, the situation can be determined by the
   ``output_active`` and ``_auto_nv`` parameters only. The ``controlled_by`` parameter
   is not needed! However, the description (enum member names) gives a quite good picture
   about the inner mechanics. If this is the case in all thinkable systems, it to be
   evaluated.



Decision
--------

Add "controlled_by" under "predefined parameters".

``"controlled_by"``:

   A drivable module indicates with this parameter, that it can be switched to be
   controlled from an other module. The datatype of such a parameter must be an
   enum. The enum keys must be names of modules or 'self'. 'self' indicates that
   the module is controlled by its own, and the value of self must be 0.
   
   The recommended mechanism is, that by changing the target of the controlling module or
   by calling its 'go' method, the module takes over control and sets the controlled_by
   parameter to its own name.


.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 22: Enable Module instead of Shutdown Command`: 022%20Enable%20Module%20instead%20of%20Shutdown%20Command.rst
.. _`SECoP Issue 48: mode parameter`: 048%20mode%20parameter.rst
.. _`SECoP Issue 59: set_mode and mode instead of some commands`: 059%20set_mode%20and%20mode%20instead%20of%20some%20commands.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
