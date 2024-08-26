SECoP Issue 64: reading multiple parameters simultaneously (closed)
===================================================================

Motivation
----------

There seems to be uses cases where a 'read multiple' command seem to be
helpful for synchroneous communication.
For async communication this is not needed.

Proposal
--------

There are more than one solution to have this.

1. new command ``"mread"``:

   The command accepts a comma separated list of specifiers <module:parameter>.
   As option, we also could use a json array of strings to specify that.

   For every specifier, the normal reply has to be expected as for a single
   ``"read"`` command. The order of replies depends on the SEC node
   implementation and might not be the same as in the ``"mread"`` command.

   This command is optional and may not be available. In this case, an error
   reply `NoSuchCommand` should be sent back.

2. extend the existing command ``"read"``:

   All things above for the ``"mread"`` command apply, but no new command
   is needed. On the ECS side, parsing of unsupported ``"read"`` command
   with multiple specifiers should distinguish between errors on modules,
   parameters or lack of multiple specifier support. In this case errors
   might be `ProtocolError`, `NoSuchModule`, `NoSuchParameter` or
   `NotImplemented`, which is slightly more complex.

3. a transaction for this

   See also `SECoP Issue 68: transaction of multiple commands`_ .

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
.. _`SECoP Issue 68: transaction of multiple commands`: 068%20transaction%20of%20multiple%20commands.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
