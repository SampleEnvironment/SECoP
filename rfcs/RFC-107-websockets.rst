- Feature: Optional WebSocket support
- Status: Final
- Submit Date: 2025-02-16
- Authors: Georg Brandl <g.brandl@fz-juelich.de>,
  Alexander Zaft <a.zaft@fz-juelich.de>
- Type: Wire
- PR: https://github.com/SampleEnvironment/SECoP/pull/33
- Version: 2.0

Summary
=======

This document describes how a SEC node exposed over TCP can optionally support a
WebSocket interface, simplifying connections from browser-based applications.


Goal
====

Browser (i.e. HTML+JavaScript) based human interface solutions are more and more
important.  Since JavaScript lacks traditional socket based APIs, sending raw
SECoP messages is not an option.  The best alternative is WebSockets (RFC
:rfc:`6455`), which are a relatively overhead-free way of exchanging messages
between two endpoints in an arbitrary pattern.


Technical explanation
=====================

After opening a connection, if the first message the SEC node receives starts
with ``GET /``, it treats the connection as a WebSocket connection, i.e. it
negotiates the connection using a prelude of HTTP requests, after which the
connection continues using the WebSocket protocol in both directions.

Since WebSockets provide reliable framing, every SECoP message is sent in a
frame.  The line ending added to separate messages over raw TCP is therefore
unneded, but remains valid.  Messages are sent as TEXT frames.

Everything else (message structure and semantics) remains unchanged.

Implementation hints
--------------------

If the SEC node doesn't want to support WebSockets, no further action is
required.  It will reply with the standard SECoP error messages, and the client
will abort the connection attempt.

A minimal implementation of the HTTP prelude is pretty small, does not have a
lot of complexity, and can be implemented even on microcontrollers `in about 200
lines of code
<https://github.com/SampleEnvironment/microSECoP/blob/master/src/http.rs>`_.


Disadvantages, Alternatives
===========================

Disadvantages
-------------

Fragmentation of SEC node frameworks into those that support WebSocket, and
those that don't.

Alternatives
------------

An alternative would be serving WebSocket over a dedicated port.  However, it
doesn't seem to offer advantages, except maybe making the implementation in
frameworks a little easier.


Open Questions
==============

None yet.
