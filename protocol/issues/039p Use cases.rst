SECoP Issue 39: Use cases(preliminary)(WIP)(needs rework)
=========================================================

Motivation
-----------
In the current specification the reasoning behind several decisions is difficult to explain.
A collection of the covered use cases and "what to do if's" may help substantially.

This issue is supposed to collect these. They will NOT be part of the specification.

Use cases: setups
-----------------

Single Request-Reply ECS:
  Use a single connection and just stick to the request-reply pattern.

Adding a Monitoring program:
  That should use a second connection and use only updates, i.e. never send change or do messages
  and as soon as possible enable updates.

Event based ECS:
  recommended: use two connections: one for playing request-reply (change, do) and the second for
  reveiving events.

What if my SEC-node is a PLC via RS232:
  You can have only one connection via RS232. Unless you use a multiplexer. like the one in frappy.

What if my setup contains of several SECoP enabled 'subnodes', but I want to make one SEC-Node:
   use frappy and set up your node using transparent clients to the other SEC-nodes.
   if needed, put filters inbetween to restrict allowed interactions.


Use cases: SEC-Nodes
--------------------

simple single threaded SEC-node via RS232 (microcontroller based?):
  difficult to get right. You MUST NOT keep the communication quiet for too long.
  best approach is a state-machine driven mainloop and everything else is attached to interrupts.
  This keeps reaction time short, however the complexity of this should never be neglected.
  USE A BIG ENOUGH INPUT BUFFER!

SEC-node on a microcontroller but with network support:
  see above. Just a tad harder because of the network stuff. also less predictable.
  use a well tested network-stack! best is still: use a more powerful empedded system.
  microcontrollers a good for selected stuff, a full-blown SEC-node isn't amongst this.
  (i.e. it may still work, but the work-to-invest/power-it-can-deliver ratio you can achieve is not good)

If we have an operating system:
  much better. if the CPU is not from the stone-age, use a python framework to get
  quick results (like frappy). Also, the OS handles all the multiple network connections for you,
  so let it do that!

multiple network connections are easy:
  Yes! So support them wherever possible.

But I can't or am unable to implement multiple network connections:
  use frappy as a multiplexer. but make your implementation FAST!

Discussion
----------
No discussion of this issue in its current form yet.
Several parts were used in meetings as basis for decision-making.
