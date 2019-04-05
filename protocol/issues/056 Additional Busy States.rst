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
