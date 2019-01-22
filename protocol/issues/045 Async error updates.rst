SECoP Issue 45: Async error updates (closed)
============================================

Motivation
----------

So far, a SEC-node can not inform the ECS about important changes, except if it is asked for the value connected.
If, for example, a parameter can suddenly no longer be read, a polling ECS get this information with the next polling request
(where the read requests result in a error reply), but an ECS using updates is NOT informed about this, though the
SEC-node gathes this information at the time of trainf to poll.

Since there seem to be no clear solution, several are proposed below.

Proposal
--------

Solution a)
+++++++++++

The ``update`` event contain ``null`` as the value in the datareport to signal to the ECS, that the value was tried to be polled,
but it didn't work. An ECS may then issue a ``poll`` request which will (if the value can still not be read)
result in an ``error`` reply containing detailed explanation.

remark:
  issue 21 needs to be re-opened or reworded as we have a use case for ``null``.

Solution b)
+++++++++++

It seems a waste to only inform the ECS about a problem, especially since the reason is already knonw by the SEC-node.
Building on Solution a), the qualifier may be extended with a field ``err`` containing the error-message, the
corresponding ``error`` reply would have contained.

The qualifier may also be extended with an ``l``\ (lastvalue) key containing the last valid value.
(Trying to keep complexity at the ECS side would speak against it, as the last valid value my also be cached there.
 On the other hand, if all clients disconnect, nobody would have this value anymore, so if it is needed to store the last valid value, the SEC-node would have to do it.)

Solution c)
+++++++++++

The ``update`` may contain the last valid value in it's data-report, but having an additional key ``error`` in the qualifier with the error-message, an ``error`` reply would have contained.

Solution d)
+++++++++++
An additional action `update with error` could be defined, returning an extended data-report.
Whether this contains ``null`` or the last valid value (or the error-msg) as the data part or inside the qualifers or in an additional field,
is to be discussed.
As this defines a new action, the fields may be reorganised here. The other solutions tried to re-use the already defined message types.


Discussion
----------
So far the motivation and the solution revolve around the use case, where a parameter can no longer be read.
It is to be discussed if there are more use cases to be covered.

not discussed in its present form.

Enrico prefers Solution b) and does not like Solution d).

Decisions from vidconf 2018-12-03
+++++++++++++++++++++++++++++++++

Based on solution (b):

whenever an async update can not determine its value, it should send an ``update`` message with a data-report containing json-``null`` and an additional qualifier ``error`` (which is only present in this case!).
The qualifier should be a modified error-report, containing the error-class instead of the origin.
