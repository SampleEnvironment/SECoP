SECoP Issue 25: Pong Format
===========================

Proposal
--------

For a more consistent syntax, the pong syntax should be:

**pong** *[id]* **[null,** \ **{"t":** *localtime* **}**\ **]**

For debugging purposes, when *id* in the ping command is omitted,
in the pong command there are two spaces after **pong**.
A client SHOULD always send an id. However, the client parser MUST treat two
consecutive spaces as two separators with an empty string in between.

With no timestamp, the pong message will contain en empty json object for the qualifiers:

**pong** *[id]* **[null,** \ **{}**\ **]**
