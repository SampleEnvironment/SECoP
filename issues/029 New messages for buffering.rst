SECoP Issue 29: New messages for buffering (closed)
===================================================

The problem
-----------
Please first read `Issue 28: Clarify buffering mechanism`_

We overload the ``change`` message with two intentions here: change a value and
buffer an intended change for later execution. This blows up code and
complicates implementations significantly.
Also it may have ambigous meanings, especially if a later revision changes the
'automagic' detection wheter a value is to be buffered or applied directly.
In addition it requires additional overhead in the case of 'normally I would want to
buffer this value, but just now I want an immediate action'.

.. _`Issue 28: Clarify buffering mechanism`: 028%20Clarify%20buffering%20mechanism.rst

Proposal
--------
Enrico proposes to introduce two new messages: ``buffer`` and ``buffered``.
These should follow the same syntax and semantix of ``change`` and ``changed``
with the exception that ``change`` ALWAYS applies the change directly, whereas
``buffer`` ALWAYS buffers the value.

If a client whishes to 'unbuffer' a value, i.e. remove the information that
another value was desired, it sends a ``buffer`` request with json-null as the
value (or an empty data part).
The SEC-Node should then remove the buffered value from its internal data
structures and reply with a ``buffered`` message, whose qualifiers do no longer
contain a buffered value.

.. note:: discussion! ``buffer`` a ``null`` value, or ``change`` to the current value?


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
Buffering of values is in the current specification (v2018-10-04) only
performed upon a ``go`` command for the target parameter, all other parameters
are unbuffered.

Examples
~~~~~~~~
buffering a value and starting later::

  > read T1:target
  < update T1:target [120,{"t":1234567890}]                    (**)
  > buffer T1:target 300
  < update T1:target [120,{"t":1234567891,"b":300.0}]          (*)
  < buffered T1:target [120,{{"t":1234567891,"b":300.0}]

  > do T1:go
  < update T1:status [[300,"ramping"],{"t":1234678900}]        (*)
  < update T1:target [300,{"t":1234678900}]                    (*)
  < done T1:go [null,{"t":1234678900}]
  < update T1:value [120.2,{"t":1234678900.2}]                 (*)

:note: (*) happens only if the module got 'activated' and will be sent on all connections which 'activated' the module.

       (**) will also be sent on all connections which 'activated' the module.



Discussion
----------

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The above proposal shall be augmented with some examples (done).
It should be an extension.
It needs further discussion.
Issue 28 also needs to be considered/updated.

Decision on the Meeting 2019-03-21
----------------------------------

Extra messages for buffering are not needed.