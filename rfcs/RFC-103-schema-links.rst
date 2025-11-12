- Feature: Linking to Schemas
- Status: Open
- Submit Date: 2024-06-18
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Protocol
- PR:
- Version: 2.0

Summary
=======

This Feature introduces a way to include links to definitions about protocol
entities in the descriptive data of a SECNode.  This is achieved by linking to
the fixed schema definitions introduced in RFC-002.


Goal
====

SECNodes should link to the specifications for their protocol entities in order
to enable three things:

- The implementors of clients can get a description of the functionality of
  these interface classes.
- A client can use the extended description to provide further functionality.
- The structure of the SECNode can be verified to follow the interface.

Protocol entities are:

- Systems
- Interface classes
- Features
- Accessibles
- Properties
- Data types


Technical explanation
=====================

Each URL is of the format: ``https://.../file.yaml``, referencing a file written
according to RFC-002.

In the SEC node descriptive data, the optional property ``schemata`` is
introduced.  It is a list of such URLs.  All given URLs should be parsed
by interested consumers (such as clients or validators) and all their
defined entities added to the library of known entities.


Disadvantages, Alternatives
===========================

Disadvantages
-------------

Specifying definitions as URLs means that they are not available if there is no
connection to the Internet.

Alternatives
------------

The schema directly in the description
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

One may include the description of the interfaces directly in the SECNodes description.
This leads to a large overhead, with JSON-formatted descriptions that have to be machine
readable and cover a large number of cases. This also basically leads to a doubling of the
structure, once being described as how the interface would look, and then how it appears.
With a valid system, these would be basically identical, not leading to any advantage.

Therefore, this does not seem sensible.

Open Questions
==============

If there are points that you know have to be discussed/solved, describe them
here, maybe with an example.
