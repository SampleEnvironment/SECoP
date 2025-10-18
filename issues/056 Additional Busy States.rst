SECoP Issue 56: Additional Busy States (closed)
===============================================

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

Use cases wich may need substates
+++++++++++++++++++++++++++++++++

In the following, use cases were collected, which might need substates or additional
commands, but let us start with the simplest use case:

A) Simple use case: Wait until a value has reached target
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

This is the simplest use case, and as it is probably also the main usage, it must be
simple: After a *change target* (or *go*), the ECS has to wait before the value has
reached target, before it is collecting some data under stable conditions.
For this use case, the simplest solution is, that the transition from
BUSY to IDLE happens when stable conditions are reached, and in the most efficient
solution, this happens as soon as possible, not waiting for example for cooling the
switch heater and ramping down the current in the leads of a superconducting magnet.

For the example of the superconducting magnet, an implementor might choose to
stay BUSY while ramping down the leads for simplicity - this is o.k., it is just
not ideal, because it is a waste of time.

B) Preparing
++++++++++++

For special cases, measurements might already happen while ramping the main value.
For this purpose, the 'prepare' command is proposed. The meaning is: bring the module
into a state, where the 'moving' of the main value can start immediately.
It seems not necessary to introduce an additional state PREPARING for that:
The status would be BUSY while preparing, IDLE afterwards. However, for being
explicit, we might introduce PREPARED, which would be a substate of IDLE.
This additional state is needed, if we define 'IDLE' as: 'the value is at target'.

C) Actions affecting the main value (Example: reference run)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In addition to the (A) and (B), there might be other actions, which we want to know
when they end. There are different types of such actions. For example a reference
run on a motor: during a reference run, the value is obviously not at target, so
it is clear, that after a command 'reference_run' the module is BUSY until the
reference run is finished, and target and value should be identical after a reference
run.

D) Actions not affecting the main value (Example: going persistent)
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Going from persistent mode to non persistent mode is an example for an action, which
does not influence the main value. The user might want to do this in order to
save time on the next field change. Or (s)he want to do the inverse to save helium.
The question is: is it is necessary for the ECS to wait for the end of such an action?
The status text might give an indication of the progress of such an action to the user,
but the ECS will normally not care. With the exception of the case, where the SEC-Node
software does not accept new actions while others are running. This is handled by the
next use case.

E) Actions blocking new commands
++++++++++++++++++++++++++++++++

If the ECS wants to initiate an action, but the SEC-Node can not yet accept it,
because it is still busy with an other action, the reply would be an 'IsBusy'
error message. The question is then: *when does it make sense for
the ECS to retry?* A sensible answer is: *whenever the status changes* or more
precise: whenever the status code *or* the status text changes. This means that we
do not need substates for this meaning, it is enough for the SEC-node to change the
status text. It is clear, that the ECS may get an IsBusy error again, but it is not
forced to try repeatedly and guess what frequency is meaningful.
Above is valid for asynchronous communication. In case of synchronous operation,
anyway a polling has to be done. Whether the client polls the status, or retries
the operation comes more or less to the same.

With this in mind, a defined BUSY substate with the meaning (3) is not really needed.
Actions which do not affect the main value, but can not be interrupted, may just be
IDLE with a different status text, or may be a custom substate of IDLE.
However, if the committee decides, that it is better to use a defined substate for that,
we might define new substate of IDLE, with the meaning (3) of busy.
However a name BUSY_IDLE or IDLE_BUSY seems not very nice. In this case the name of
the substate should be chosen by the implementor, for example CHANGING_PERSISTENCY.

F) Influencing the end of the BUSY phase
++++++++++++++++++++++++++++++++++++++++

Some users may want to influence the criterium for the end of BUSY phase.

For example:

1) During cooldown of the superconducting switch, the magnetic field might
   still oscillate slightly, so the user wants to wait for this before
   measuring.

2) Not really a sample environment issue, but otherwise a good example: the user
   wants to wait until air cushions have switched off, because the beam geometry
   is affected sligthly.

3) Tolerance and window of temperature

Instead of additional substates, the SEC-node may offer one or several additional parameter(s),
influencing, when the transition to IDLE happens. This is already the case in
example (3) above. For the example of motors with air cushions, this might be a parameter
settling_time, defining how long to wait after the air cushion was switch off. Or, for a
magnet, it might be a parameter "complete_on" with the values "field_at_target",
"switch_closed" and "leads_at_zero".

The naming and meaning of these parameters may be defined for specific interface classes,
but otherwise we should not try to find a more generic meaning.

The advantage of this approach is, that it is "self documented", by the selection of
the parameter names, and the description of the parameter.

A disadvantage is, that the criteria are preselected and then valid for all clients,
they can not be chosen on the fly for different clients. But do we need that really?


Decision
--------

.. table:: Useful statuscodes

     ====== ================ ========== ============== =========================================
      code   name             generic    variant name   Meaning
     ====== ================ ========== ============== =========================================
         0   DISABLED         DISABLED   Generic        Module is not enabled
       100   IDLE             IDLE       Generic        Module is not performing any action
       130   STANDBY          IDLE       Standby        Stable, steady state, needs some preparation steps,
                                                        before a target change is effective
       150   PREPARED         IDLE       Prepared       Ready for immediate target change
       200   WARN             WARN       Generic        The same as IDLE, but something may not be alright,
                                                        though it is not a problem (yet)
       230   WARN_STANDBY     WARN       Standby        -''-
       250   WARN_PREPARED    WARN       Prepared       -''-
       300   BUSY             BUSY       Generic        Module is performing some action
       310   DISABLING        BUSY       Disabling      Intermediate state: Standby -> **DISABLING** -> Disabled
       320   INITIALIZING     BUSY       Initializing   Intermediate state: Disabled -> **INITIALIZING** -> Standby
       340   PREPARING        BUSY       Preparing      Intermediate state: Standby -> **PREPARING** -> PREPARED
       360   STARTING         BUSY       Starting       Target has changed, but continuous change has not yet started
       370   RAMPING          BUSY       Ramping        Continuous change, which might be used for measuring
       380   STABILIZING      BUSY       Stabilizing    Continuous change has ended, but target value is not
                                                        yet reached
       390   FINALIZING       BUSY       Finalizing     Value has reached the target and any leftover cleanup operation
                                                        is in progress. If the ECS is waiting for the value of this
                                                        module beeing stable at target, it can continue.
       400   ERROR            ERROR      Generic        An Error occured, Module is in an error state,
                                                        something turned out to be a problem.
       430   ERROR_STANDBY    ERROR      Standby        An Error occured, Module is still in Standby state,
                                                        even after ``clear_errors``.
       450   ERROR_PREPARED   ERROR      Prepared       An Error occured, Module is still in PREPARED state,
                                                        even after ``clear_errors``.
     ====== ================ ========== ============== =========================================

