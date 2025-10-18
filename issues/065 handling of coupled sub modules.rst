SECoP Issue 65: handling of coupled (sub)modules (closed)
=========================================================

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

Thoughts after the video meeting 2022-01-26
-------------------------------------------
by Markus

1) what is the goal of issue 65?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- standardise the mechanism to switch between interlinked modules

more precise:

we have a group of interlinked module, only one of them could take over control.

* we want to define a mechanism to switch modules
* we want to define how this is indicated

2) proposed solution
~~~~~~~~~~~~~~~~~~~~

* setting the target of a module switches to this module
* the 'controlled_by' parameter indicates which one

3) ambiguousness
~~~~~~~~~~~~~~~~

The question is: To which modules belongs the controlled_by parameter?
There are two possible interpretations of the proposed issue:

1. Each module of the group has a controlled_by parameter and each of these parameters
   contains an enum value corresponding to the module in charge.
   This is as far as I understand, Klaus interpretation.

2. From the inner logic, it can be seen, which modules is controlling (e.g. the
   one keeping a PID controller) and which module is controlled. The 'controlled_by'
   parameter should be on the module potentially controlled by the other(s). This
   is the point of view of Markus and Enno.

Both interpretations have disadvantages. Let us start with (2).

What should we do, when a hierarchy (controlling - controlled) is missing,
i.e. in a symmetric case? Example: V and I on a laboratory power supply,
where setting the target of V or I makes the other to act like a Readable.
As this case is symmetric, we do not know where to put the 'controlled_by'
parameter. We might specify that in this case it is allowed to put this
parameter to both modules, accepting the cost of redundancy, or we might
let the implementor choose an arbitrary module. Not a big problem.

Now to the disadvantages of interpretation (1):

Redundancy:

   It is not needed to carry the information on all modules. For example
   with 3 modules T_sample, T_reg and P_heater, where one may choose to
   regulate on one of the temperatures, or use manual power. We have 3
   states, and this information can be taken from only one of the 'controlled_by'
   parameters, the others are redundant.

More complex cases:

   T_reg is controlling 2 modules: P_heater and pressure_nv. We have two overlapping
   interlinked groups: (T_reg, P_heater) and (T_reg, pressure_nv).

   Each group needs an information about the controlling module.

   If we place the 'controlled_by' Parameter only on P_heater and pressure_nv,
   the case is clear. But as T_reg is part of two groups of interlinked
   modules, it is not clear which information exactly T_reg:controlled_by
   should carry. It is better to omit this parameter, it can deliver only
   redundant information and its content is not well defined.

A good question is, if such a case is too complex for the feature.
What exactly should be done when T_reg takes over control or when control
is taken from T_reg? In this system, a solution might be that there is a
boolean parameter T_reg:_auto_nv deciding whether pressure_nv is used for
control or not. Setting a target on a module is always grabbing control,
but there might be side effects. For example:

* setting T_reg:target leads to: P_heater.controlled_by := 'T_reg'; if T_reg._auto_nv: pressure_nv.controlled_by := 'T_reg'
* setting P_heater:target leads to: P_heater.controlled_by := 'self'; pressure_nv.controlled_by := 'self'
* setting pressure_nv:target leads to: pressure_nv.controlled_by := 'T_reg'; T_read:_auto_nv := False

This behaviour is driven by fact, that this implementation has not foreseen
to control the temperature solely by the needle valve.

We could see that even with this implemented dependent behaviour,
the goals are followed: by setting <module>:target, <module> takes
over control within its group, and all information about the control
dependency is present. 'output_active' as a parameter
is only needed, when we want to deactivate control completely.
I realise now, that this might be done with an additional enum member
'NONE', as Klaus proposed.

4) an other proposition
~~~~~~~~~~~~~~~~~~~~~~~

Thinking about it, I consider changing again completely the behaviour
respecting the principle of least surprise. Let us assume above system
in the mode where T_reg is in auto nv mode, controlling the pressure_nv.
What might the user expect when setting pressure_nv:target?
With the current proposal, the automatic needle valve control is switched
off automagically. Is this really what he expects? We should also avoid that
changing the target would only temporarely changing the needle valve setpoint
and the control loop would overwrite it shortly afterwards.
An good solution would be: raise an error, telling that pressure_nv is
controlled by T_reg and can not be changed manually. This seems
to be the least surprise, when a sensible error text is shown.
In this case we would need another mechanism to select the
controller -> output relation. Either by making controlled_by writable
or by doing it the other way: having two writable boolean parameters
T_reg:control_P_heater and T_reg:control_pressure_nv for this, named
after the output modules.

5) naming
~~~~~~~~~

'controlled_by' seems to be misleading. A PID loop may control a temperature
using power as the output variable. Here the temperature is controlled by the
PID loop, and not the heatoer power by the temperature. Other propositions:

* 'output_of' - not applicable for the mentioned V/I power supply example
* 'locked_by' - meaning is: changing the target is locked, because the
  mentioned module is using it. Again an enum, with names of realted modules,
  but with the special value 'unlocked' instead of 'self'.

Using the meaning 'locked_by', we still might allow side effects of changing
target. In the example we might have a parameter 'locked_by' only on P_heater
and pressure_nv, and changing T_reg:target may switch to controlled mode, which
is probably intended, while changing P_heater:target might be prohibited
because it is locked. With this model, the implementor can choose the
behaviour adapted best to the use case. For the above mentioned V/I power
supply, automatic switching might also be expected. In this example
an 'active' parameter might be more suitable.


Propositions of vidconf 2022-03-01
----------------------------------

We identified two different things the ECS wants to be informed about:

a) a module might be in a state, where the value of the target is inactive,
   i.e. has no more influence on the behaviour of any module

b) the target of a module might be the output of an other module

In addition, we want to describe, how the coupling between modules may
be changed.

We agree that both should be possible:

1) changing the target of a module might switch to make its target active,
   by uncoupling its target from the output of a leader module, or by
   coupling its own output to the target of a controlled module.

2) changing the target of a module might complain when its target is not
   active or when its target is the output of a leader module.

The ESS should be informed whether (1) or (2) is implemented.


predefined behaviour / naming in SECoP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the discussion we thought about a parameter 'linked_outputs',
being a struct with member names being modules, and member values
are a boolean indicating whether the module is linked or not.

Further evaluation discovered, that coming back to a parameter like
'controlled_by' is better, because no ambiguity is possible:
as it is not allowed to link outputs of two controller modules to
one controlled module at the same time. Instead of 'controlled_by'
'linked_input' is also proposed as name.

Even outside a controller - controlled module relation, it is possible
that a target is getting inactive. For this reason it is proposed to
have an additional 'control_active' boolean parameter for this.

Example: a power supply with 2 modules voltage and current.
Setting the target of one of them would set its own control_active
parameter to 'true' and the other to 'false'.

Instead of an additional 'control_active' parameter, it might be
an alternative to add an addition item "CONTROL_INACTIVE" to the 'mode'
parameter. However, the mode parameter is a writable, and the current
state is reflected by the status, so we would need an additional
predefined value for the status code. As a consequence, we would need to
add an other status 'CONTROL_INACTIVE' which could either be a substate of

1) 'IDLE' (reasoning: behaviour like a Readable)
2) or 'WARN'(reasoning: it is a somehow broken Drivable)

An indication, whether changing target has the effect of changing coupling
of modules, is given by the presence of 'control_active' in the 'influences'
property of 'target'. If this is not present, the coupling may be changed
only by setting the 'control_active' or 'linked_input' parameter.

Finally instead of 'linked_input' Markus proposes coming back to 'controlled_by'.



Decision
--------

Add ``controlled_by`` and ``control_active`` under the list of "predefined parameters".

``"controlled_by"``:
   Module might be coupled by a input - output relation. A input module
   (Drivable or Writable) might be controlled by an other module, linking an output
   of the module to the target of the input module.
   The datatype of the ``controlled_by`` parameter must be an enum, with the names being
   module names or ``self``. The enum value of 'self' must be 0.
   A module with such a parameter indicates, that it may be the input of one of the named modules.

   The recommended mechanism is, that a module takes over control by sending a target
   change or a ``go`` command. Before receiving the reply, the ``controlled_by`` parameter
   of the input module is set to the controlling module, or to ``self``, if the
   target of the controller module itself is set.
   In case a module may have several outputs, additional parameters may be
   needed for switching on and off control of individual input modules.

``"control_active"``:
   A flag indicating whether a drivable or writable module is currently active.
   On a drivable without control_active parameter of with
   control_active=True, the system is trying to bring the value to the target.
   When control_active=False, this control mechanism is switched off, and the target value
   is not considered any more.
   For example a controlling module ``control_active`` parameter is false, when the controlled
   modules ``controlled_by`` parameter is set to ``self`` (or to an other module).
   But ``control_active`` might also be needed when two Writable modules depend on each
   other in a system where not both may be active at the same time. An example would be
   a power supply with two writable modules 'current' and 'voltage': On the controlling
   module control_active=true and the target parameter is used for the control quantity.
   The other module (control_active=false) acts like a Readable, its target parameter is
   ignored. Changing the target value of the latter would switch control from one module
   to the other, toggling the control_active parameter of both modules.
   
   In addition, we should try to make sections for the predefined parameters (and parameters).

   But how? parameters and commands together?

   - readable basics: value, status
   - drivable: target, status (BUSY), go(), stop()
   - modes: mode, hold()m shutdown()
   - error handling: reset(), clear_error()
   - polling: pollinterval
   - ramping: ramp, setpoint, time_to_target
   - communication: communicate
   - coupled modules: controlled_by, control_active


Appendix (MZ 2023-06-30)
------------------------

In the current draft spec (as of May 2023), it is not clear what to do for
switch off control when a controller module has no output module.

In this case it is proposed to habe a command control_off to switch off control.
For consistency, this may also be offered when an output module is present,
the behaviour is module dependent.

control_off is typically setting the controller to a mode which is 'saving energy':
e.g. active heating and cooling off, or in the case of a motor switching the motor
current off.


.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 22: Enable Module instead of Shutdown Command`: 022%20Enable%20Module%20instead%20of%20Shutdown%20Command.rst
.. _`SECoP Issue 48: mode parameter`: 048%20mode%20parameter.rst
.. _`SECoP Issue 59: set_mode and mode instead of some commands`: 059%20set_mode%20and%20mode%20instead%20of%20some%20commands.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
