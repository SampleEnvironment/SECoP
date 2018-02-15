SECoP Issue 15: Support for Start/Stop/Pause/Shutdown Commands
==============================================================

Initially, only one command was predefined:

  * stop

From the ILL software group it is a wish to support the following
commands:

  * start
  * pause
  
In addition the following command was proposed

  * shutdown (meaning: go to safe state after finishing the experiment)
 
Discussion
----------

* What is the exact meaning of these commands?
* What is the difference of *stop* and *pause*?
* Is an interplay possible with the 2 worlds using/supporting or not the commands *start* and *pause*

Decision
--------

At the meeting 2018-02-12 in Grenoble we agreed about the following commands:

go
..

Optional on a drivable. If present, the 'go' command is used to start the
module. If not present the module is started upon a change on the target
parameter.

hold
....

Optional command on a drivable. Stay more or less where you are, cease movement, be
ready to continue soon, target value is kept. Continuation can be trigger with 'go',
or if not present, by putting the target parameter to its present value.

abort
.....

Optional command.
Stops the running module in a safe way (for example switches the heater off).

*Question: May or should the 'target' parameter be changed?*

stop
....

Mandatory command on a drivable. Cease movement, set the target parameter to a value
close to the present one. Act as if this value would have been the initial target.

reset
.....

Optional command for putting the module to a state predefined by the implementation.

shutdown
........

Optional command for shuting down the hardware.
When this command is sent, and the triggered action is finished (status in idle mode),
it is safe to switch off the related device.

*Question (Markus): I guess setting the target value or issuing the 'go' command
will cnacel the shutdown state. This is not visible any more. Do we not need an additional
status 'shutdown'?*

*Question (Markus): Should 'shutdown' try hard to success in any case, i.e. 'stop' first,
if status is 'busy'?*



