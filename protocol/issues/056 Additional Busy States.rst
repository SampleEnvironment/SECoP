SECoP Issue 56: Additional Busy States (under discussion)
=========================================================

Motivation
----------

This issue is raised when discussion the possible substates of a cryomagnet with persistency.
After reaching the target field, in principle a measurement can start, but the action
still has to continue, running down the current in the leads. The question arised, how
to deal with this. Is this BUSY or IDLE? Should we introduce a new substate for this?

Proposal
--------

The proposal of Klaus is, to introduce FINALIZE, which is a substate of BUSY.
If the ECS sees that a module has this state, it might offer the user the
possibility to choose wheter to wait or not for this state to be finished.


Discussion
----------

Markus proposed, that this should be handled differently. If the SEC-Node
programmer wants to offer the possibility to choose, (s)he implements a
parameter named like "finalize_is_busy", which lets the user choose.
Actually, such situations are handled differently at different facilities:
At MLZ, measuring contimues only after finalizing, at PSI/SINQ, the
measurement continues and ramping down the leads is done in background.

Markus thinks, that a more general discussion of the BUSY state has to be
done before this decision. BUSY may have different meanings:

1) waiting for value to reach target
2) an action is not finished
3) we have to wait before we can do the next action

Markus sees the BUSY status mainly with meaning (1). Meaning (2) seems not relevant
for the ECS: as long as the SEC-Node can handle to accept new actions while
an other is still running, nobody has to care. As there are SEC node developers
which allow to change the target while BUSY, meaning (3) can also not be the
main maining of BUSY. We discussed that when an action is started while
something is still running, an the SEC-node can not handle this, it must
report an error message with the error class "IsBusy".

What we did not define, is, if BUSY has only the meaning (1) or if it is a logical
OR of (1) and (3). In the first case, an "IsBusy" error might be the reply of
an action (change target or command), even when the status is IDLE.

If we want be very clear with this, we may need an additional boolean parameter "running",
which only tells that the value on the way to the target. status=BUSY would then have
meaning (3).

If the committee does not like coming back to the solution of the "finalize_is_busy"
parameter, Markus proposal for a compromise would be:

- status=BUSY is logical OR of meaning (1) and (3)
- we leave it open to the SEC node developer to offer a "finalize_is_busy" parameter or
  to use a newly defined substate EARLY_IDLE=101 (substate of IDLE!).

EARLY_IDLE means then:
    target is reached AND ready to accept an other "change target".

We may in addition introduce FINALIZE=301 (substate of BUSY!), which means:
    target is reached, measurement might continue, but 'change target' is not yet accepted.


Additional considerations after vidconf-2019-04-11
--------------------------------------------------

Preparing
+++++++++

We decided to define a 'prepare' command. The meaning is: bring the module
into a state, where the 'moving' of the main value can start immediately.
I think this is undisputed. It seems not necessary to introduce an additional
state PREPARING for that.


Use case for meaning (3)
++++++++++++++++++++++++

In the discussion on vidconf-2019-04-11 it was not clear if the ECS needs to
know the status in the sense of meaning (3).

It is clear, that the ECS can always try to do any action, and the SEC node can
reply with an 'IsBusy' error in this case. The question was: **when does it make sense for
the ECS to retry?** A sensible answer is: **whenever the status changes** or more
precise: whenever the status code **or** the status text changes. This means that we
do not need substates for this meaning, it is enough for the SEC-node to change the
status text. It is clear, that the ECS may get an IsBusy error again, but it is not
forced to try repeatedly and guess what frequency is meaningful.


Definition of BUSY
++++++++++++++++++

Therefore let us define BUSY not as logical OR of (1) and (3), but:

* the status is BUSY, whenever an action is not yet 'completed'.
* 'completed' is defined by the implementor of an action, but in the following sense:
    - A 'change target' action is completed, when the value reaches the target, and
      when there influence of this action on the experiment has stabilized.
      The crucial point here: both criteria above depend on some tolerance
      threshold.
    - An action might be finished, but still some process triggered by this action may
      be under way, if this process is not hindering other actions to be performed.


Now a SEC-Node implementor might want to offer a possibility to influence the criteria
for an action to be 'completed'. Still I want to compare two possibilities:


Substates
+++++++++

We might specify a substate FINALIZING=301 and/or EARLY_IDLE=101, and then the ECS
might decide, how to deal with that. Possible options for an ECS (using SICS as
an example):

Existing commands:

* ``drive <module> <target>`` means: change target and wait for IDLE
* ``run <module> <target>`` means: change target and do not wait

New commands or command options:

* ``drive -quick <module> <target>`` means: change target and wait for IDLE or FINALIZING
* ``drive -extended <module> <target>`` means: change target and wait for IDLE after EARLY_IDLE

The advantage of this approach is, that one can decide 'on the fly', which mode to
use, and it is not dependent on the setting of a parameter, which somebody has set
at some time. The downside is: You have to modify the 'drive' command, which is
quite a big thing.

The other approach is to introduce a new parameter ``drivemode`` on the ECS side, with
values (0: quick, 1: normal, 2: extended), which can be preset for all further drive
commands on that module.

<module> drivemode 1

We could also imagine to have a global parameter on the ECS, which is influencing
the drive command of all modules.

I think the main problem of both variants of this approach is, that we have no easy
possibility to document what these substates mean for a specific module.


Extra Parameters on the Module
++++++++++++++++++++++++++++++

Instead of additional substates, the SEC-node offers one or several additional parameter(s),
influencing, when the transition to IDLE happens. This is already the case on
some temperature modules, with the window/tolerance parameters. For the example
of motors with air cushions, this might be a parameter settling_time,
defining how long to wait after the air cushion was switch off. Or, for a
magnet, it might be a parameter "complete_on" with the values "field_at_target",
"switch_closed" and "leads_at_zero".

The naming and meaning of these parameters may be defined for specific interface classes,
but otherwise we should not try to find a more generic meaning.

The advantage of this approach is, that it is "self documented", by the selection of
the parameter names, and the description of the parameter.

A disadvantage is, that the criteria are preselected and then valid for all clients,
the can not be different for different clients. But do we need that really?
