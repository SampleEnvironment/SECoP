- Feature: Equipment ID in Describing Message
- Status: Rejected
- Submit Date: 2017-05-30
- Authors: SECoP committee
- Type: Issue
- PR:
- Version: 1.0

Summary
=======

Could the equipment ID go into the "specifier" field of the "describing"
message?


Issue text
==========

The equipment ID is a SEC node property, and it is therefore redundant
to put it as the second item of the describe message.

However as the describe/describing message might be extended later, for
example to get the description of single modules only, we should specify
a fixed word for the second item of the describe message, for example the
keyword "ALL" or "All".

At the meeting in Berlin (2017-05-30) this was discussed, but it was not
yet decided the the keyword should be exactly. Until a final decision,
SECoP clients should ignore the second item.

Opinions
--------

We should use key keyword ALL (Markus Zolliker).

Decision
--------

The decision was taken to use a bare period as placeholder:

.. code::

    describing . {"modules": ...
