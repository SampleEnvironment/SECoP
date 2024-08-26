SECoP Issue 16: wanted_target Parameter (closed)
================================================

From the minutes of the meeting on 27 november 2017:

create an Issue (to be discussed) for:
reading the (RO) target parameter gives you the HW value

if there is no start command available, writing to the (RW) wanted_target starts the action else you need to call start() after writing to wanted_target. In any case, the target parameter reflects the value used by the hw.

Decision
--------
Lutz thinks that looking at the status (and predefining a few values for it) may be sufficient and to have an additional parameter ‘wanted_target’ can be avoided.

Rejected (decision meeting 2018-02-13 in Grenoble)
