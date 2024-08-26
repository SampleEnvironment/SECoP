SECoP Issue 68: transaction of multiple commands (closed)
=========================================================

Motivation
----------

This proposal is meant to supplement issue `SECoP Issue 64: reading multiple parameters simultaneously`_
with a possibility to execute more than one command at the same or almost
the same time, if the SEC node and the hardware supports this.

Proposal
--------

We need to implement one (three) new commands to specify a transaction of
multiple commands, which could be executed as one block. With proper support
of the hardware, this could allow to read or change parameters at the same
time.

A new command ``"transaction"`` with different specifiers is used for this.
The SEC node may reply with an `NoSuchCommand` error, if it cannot handle
transactions.

Inside a transaction, one type of command is allowed only. Valid commands
inside may be ``"read"``, ``"change"`` and ``"transaction"``. You may not
mix the commands ``"read"`` with ``"change"`` and a `NoMixedTransaction`
error error code is replied.

If the SEC node supports transactions, the specifiers could be:

``"start"``:

     Next commands are validated and stored for execution, but not executed.
     The reply is ``"transaction started"``. This could be extended with
     information, how many transactional space is available (count and/or
     byte space), which has to be discussed.

     After validation of ``"read"`` or ``"change"`` commands, there are
     replied with a proper ``"error"`` or ``"transaction continue"``.
     If a case of an error, the last command is not stored, but the
     transaction keeps open.

     It is not allowed to nest transactions. This is an error and should
     be replied with `NoNestedTransaction` error code.

``"commit"``:

     All stored (validated) commands are executed. The implementation
     should do it's best, to execute all commands at same or almost same
     time. It is the decision of the SEC node implementor, to change the
     order of the stored commands.

     All commands have their normal replies, as they would come without
     transaction. After all commands are executed, the command reply is
     ``"transaction committed"``.

     A new transaction could be started after reply of this command.

``"cancel"``:

     All stored (validated) commands removed and not executed.
     The command reply is ``"transaction cancelled"``.

     A new transaction could be started after reply of this command.

.. code::

  > transaction start
  < transaction started {"maxcommands":5,"maxbytes":256}
  > read mymod1:target
  < transaction continue {"maxcommands":4,"maxbytes":236}
  > read mymod12:value
  < error mymod12:value ["NoSuchModule",...]
  > change mymod2:target
  < error mymod2:target ["NoMixedTransaction",...]
  > read mymod1:value
  < transaction continue {"maxcommands":3,"maxbytes":216}
  > transaction commit
  < reply mymod1:value ...
  < reply mymod1:target ...
  < transaction committed


Alternative Proposal by MZ
--------------------------

A transaction should be handled in one command, so that the SEC node does not
need to build a structure for remembering all the actions to be performed.

In this approach, a transaction is composed by a list of actions to be performed
'at once' meaning that no other messages are handled in between.

In addition, this approach allows to check that a parameter value has not changed
since the last read to the client.

In the following examples, the messages should be on one line, the line breaks
are for readability only.

.. code::

    transaction . [
      ["get", "mod1:target", [3.5, 4.5]],
      ["change", "mod1:ramp": 5],
      ["change", "mod1:target": [3.5, 5.5]],
      ["read", "mod1:value"],
      ["read", "mod1:status"],
      ["read", "mod1:_raw_value"]
    ]

    result . {
      "mod1:target": [[3.5, 5.5], {"t": 123142456.7}],
      "mod1:value": [[3.401, 4.51], {"t": 123142456.1}],
      "mod1:status": [[300, "ramping"], {"t": 123142456.7}],
      "mod1:_raw_value": [null, ["HardwareError", "no raw value available", {}]]
    }


Rules:

- during handling of a transaction, no other messages are handled.
- first all "change" actions are checked for validity. if this fails
  a transaction_error of the first form is returned and the handling
  of actions is skipped
- the individual actions are executed in order
- a read action reads the value of the given parameter from hardware and
  stores it including qualifiers in the result object
- a get action stores the cached value including qualifiers in the result object
- if a get or read action contains three items, instead of storing the value, it
  is checked against the given last item of the action. if these values do not match
  a transaction_error of the seconds form is returned and the handling of further
  actions is skipped
- a change action changes the given parameter and saves the replied
  value including qualifiers in the result object. if this fails a
  transaction_error of the second form is returned and the handling of further
  actions is skipped
- if a read or get action fails, [null, <error report>] is stored in the result object
- finally the result is returned in the result reply

First form of transaction error, summarising all range checks:

.. code::

    transaction_error . {
      "mod1:target": ["RangeError", "5.5 is not within 0..5", {}]
      "mod1:ramp": ["RangeError", "5 is not within 0..3", {}]
    }

Second form of transaction error, for the first action failing

.. code::

   transaction_error "mod1:target" ["CheckFailed", "[3, 4.5] does not match [3.5, 4.5]", {}]

   transaction_error "mod1:ramp" ["CommunicationFailed", "no reply", {}]





Discussion
----------

Markus raised the question of the use cases. Enno did not see the need. Klaus
statet that, after having ``influences`` property, the issue needs to be re-
thought. Markus pointed out that bundling of information is already possible and
should be used in such cases. Enno figured out that data duplication could be
avoided in many cases, as we have access to individual elements on structured
data types already.

Decision
--------

Agreement on this: new SECoP rule:

- If multiple items need to be accessed simultaneously, put them into a
  structure data type.
- If a write triggers side-effects, use the ``influences`` property
  to point this out.

.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 64: reading multiple parameters simultaneously`: 064%20reading%20multiple%20parameters%20simultaneously.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
