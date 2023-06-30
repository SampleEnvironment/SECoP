SECoP Issue 80: issues with software ramp (proposed)
====================================================

Motivation
----------

Programming a software ramp, I was not sure from where a ramp has to be started.
Starting from the current value is not nice when continuing a ramp just after
an other one, and the current value lags a little bit behind the ramping setpoint.
Starting from the current target or setpoint is not nice when the target
or setpoint was not active.


Proposal
--------

1) It would be helpful if it would be possible to know if the previous
   setpoint was 'active' or not. Possible Solutions:

   a) Modules with a control loop (candidates for a software ramp) need a control_active
      parameter, even not in the context of coupled submodules.

   b) define an error class 'Undefined', which is reported on read and update messages
      in case the setpoint (and target) is not active.
      If this mechanism is used on the target parameter, this may replace the
      control_active parameter in all cases.

   c) add an 'allow_null' property to datainfo adding JSON null to the allowed values of
      a data type. Also a candidate for replacing the control_active parameter.
      

2) A user which wants explicitly doing a ramp from A to B can do:

change T:ramp_enable False
change T:target <A>
change T:ramp_enable True
change T:target <B>

In order for this to work, the implementation must be able to handle correctly
the two latter messages while the module is busy.

It would be much nicer to do:

change T:setpoint <A>   # changing the setpoint immediately to <A>
change T:target <B>   # starting the ramp from <A> to <B>

This would need to change the predefined 'setpoint' parameter to be readonly=True.


Disucssion
----------

At the vidconf 2023-06-07 it was decided to go with i) for 1).

(2) was not yet discussed
