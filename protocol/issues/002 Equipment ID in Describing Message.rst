SECoP Issue 2: Equipment ID in Describing Message (under discussion)
====================================================================

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

We should use key keyword ALL (Markus Zolliker)

Decision
--------
