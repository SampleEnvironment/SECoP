SECoP Issue 29: New messages for buffering
==========================================

The problem
-----------
Please first read `Issue 28: Clarify buffering mechanism`_

We overlay the ``change`` message with two intentions here: change a value and
buffer an intended change for later execution. This blows up code and
complicates implementations significantly.
Also it may have ambigous meanings, especially if a later revision changes the
'automagic' detection wheter a value is to be buffered or applied directly.
Also it requires additional overhead in the case of 'normally I would want to
buffer this value, but now I want an immediate action'.

.. _`Issue 28: Clarify buffering mechanism`: 028p%20Clarify%20buffering%20mechanism.rst

Proposal
--------
Enrico proposes to introduce two new messages: ``buffer`` and ``buffered``.
These should follow the same syntax and semantix of ``change`` and ``changed``
with the exception that ``change`` ALWAYS applies the change directly, whereas
``buffer`` ALWAYS buffers the value.

If a client whished to 'unbuffer' a value, i.e. remove the information that
another value was desired, it sends a ``buffer`` request with json-null as the
value (or an empty data part).
The SEC-Node should then remove the buffered value from its internal data
structures and reply with a ``buffered`` message, whose qualifiers do no longer
contain a buffered value.

Also, ``buffered`` will send the same data to the client as ``changed``,
including the new buffered value in the qualifiers.
If a client requests buffering on a parameter, the SEC-Node does not support
buffering on, an appropriate error message should be sent.
Otherwise the same mechanics as for ``change`` apply, i.e. trying to ``buffer``
a value not matching the datatype of the parameter MUST be replied with an
BadValue error (as in the ``change`` request).

If the existence or value of a buffered value is changed (via the ``buffer``
message), an ``update`` message MUST be send to all clients subscribed to this
parameter.

The introduction of these new messages does not influence the usage or
usefulness of the buffered value.
Buffering of values is in the current specification (v2018-06-14) only
performed upon a ``go`` command for the target parameter, all other parameters
are unbuffered.

Discussion
----------
so far none.
