SECoP Issue 68: transaction of multiple commands (unspecified)
==============================================================

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


Discussion
----------



Decision
--------

.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 64: reading multiple parameters simultaneously`: 064%20reading%20multiple%20parameters%20simultaneously.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
