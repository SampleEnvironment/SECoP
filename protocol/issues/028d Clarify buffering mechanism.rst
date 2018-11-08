SECoP Issue 28: Clarify buffering mechanism
===========================================

The buffering mechanism
-----------------------

Currently (`V2018-06-14`_), the following is specified;

The following commands are predefined (extensible):

-  ``go`` optional on a drivable. If present, the 'go' command is used to start the
   module. If not present the module is started upon a change on the target
   parameter.

-  ``hold`` optional command on a drivable. Stay more or less where you are, cease
   movement, be ready to continue soon, target value is kept. Continuation can be
   trigger with 'go', or if not present, by putting the target parameter to its
   present value.

-  ``abort`` optional command. Stops the running module in a safe way (for example
   switches the heater off).

-  ``stop`` mandatory command on a drivable. Cease movement, set the target parameter
   to a value close to the present one. Act as if this value would have been the initial target.

In the current part of the documentation (`V2018-06-14`_) it is not
explicitly stated, how the buffering of the intented target value is to be handled
for the above mentioned commands.

.. _`V2018-06-14`: ../secop_v2018-06-14.rst#commands



Proposal
--------
Enrico + Frank propose to introduce an additional qualifier 'b' which represents
the currently buffered value, if any.
This qualifier it will only be present in ``changed`` or ``update`` replies from
the SEC-Node if there is actually a value buffered.
This buffered value SHOULD be required to reflect the value the target value will
have, after it was read back from the hardware,
e.g. all roundings/clippings etc. are to be applied already.
This is true even if the 'value to be buffered' equals the current target value.

Currently, buffering of the target parameter is automatically active, if the
module has an interface class of at least Writable and has a ``go`` command implemented.
In this operation mode, a change of the target parameter gets buffered,
as the target parameter is required to reflect the current target value from the hardware.
(This is especially important if the hardware does not support buffering.)
The ``changed`` reply then already contains the buffered value in the qualifiers.
Also, an ``update`` event (with the new qualifier) should be send to all other
active connections having activated updates for that parameter.

:note: The ``update`` events should be sent before the reply to the buffering query is sent.


Behaviour of ``go``/``hold``/``abort``/``stop`` commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  ``go`` optional on a drivable. If present, the 'go' command is used to start the
   module. Changes to the target parameter get buffered and reflected in the qualifiers.
   Only if the 'movement' is done, the buffered value is removed, i.e. the parameter target
   reflect again the current and the intended target from the hw.
   This should also trigger update events on all enabled connections.
   If ``go`` is not present the module is started upon a change on the target
   parameter.

-  ``hold`` optional command on a drivable. Stay more or less where you are, cease
   movement, be ready to continue soon.
   If a value was buffered for target, the target may reflect the current position,
   otherwise it is kept. The buffered value is untouched by this command.
   Continuation can be triggered with 'go', or if not present, by putting the target
   parameter to its present value.

-  ``abort`` optional command. Stops the running module in a safe way (for example
   switches the heater off).
   note: this SHOULD NOT clear the buffered value.

-  ``stop`` mandatory command on a drivable. Cease movement, set the target parameter
   to a value close to the present one. Act as if this value would have been the initial target.
   If a value was buffered for target, it is cleared.
   This should also trigger update events on all enabled connections.


Problems
~~~~~~~~
See `Issue 29: New messages for buffering`_

.. _`Issue 29: New messages for buffering`: 029p%20New%20messages%20for%20buffering.rst


Discussion
----------

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Will be updated once Issue 29 is sorted out.

Keep as 'under discussion' for now.

