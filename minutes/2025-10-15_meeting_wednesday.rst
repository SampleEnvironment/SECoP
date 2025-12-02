SECoP in person workshop at ESS - Wednesday
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.. sidebar:: participants

     * Georg Brandl
     * Peter Braun
     * Niklas Eckström
     * Enrico Faulhaber (remote)
     * Klaus Kiefer
     * Bastian Klemke (remote)
     * Alexander Zaft
     * Markus Zolliker

.. contents:: Agenda
    :local:
    :depth: 3



1) Introduction, Demonstrations
===============================

Niklas welcomes everyone at ESS.

The minutes of last meeting are approved (with slight modifications from Georg)
and merged.

Spin
----

Georg gives a short demonstration on SPIN, a tool for HMI for SE racks at MLZ:

* web-based, svg+html. messages through websockets
* Can connect to different protocols to read values

Thorsten Bögershausen joins

Bastian Klemke joins online

* Georg walks us through the different features of SPIN
* available on the MLZ forge, GitHub mirror can be discussed later when needed
* Documentation is available here: https://forge.frm2.tum.de/public/doc/spin/master/

SECoP-Service
-------------

Peter shows his work on a "SECoP-Service" providing a Sample Environment
web-gui built in the context of the ROCK-IT project

* Generic GUI generated from the SECoP description
* Similar principle to frappy, but easy if it has to be shown on multiple
  computers, since setup has to happen only once
* implemented in elixir
* has a database for saving historic data
* it is probably a bit heavy for using as the "simple client" goal for
  outreach, since it needs to be deployed/administered.
* The code is available at https://github.com/Bilchreis/secop_service

Klaus shares some history of the ROCK-IT project

1.5) Specification, Committee Work
==================================

Recap: Current Status and Open Issues
-------------------------------------

Georg opened a PR for including the approved Acquisition RFC into the
specification.

From the open issues, most will be discussed in their own sessions:

  * NeXus (Operando4NeXus on Friday)
  * Machine readable spec (Tomorrow)
  * rewrite of the website introduction (Tomorrow)

Issue 13 had no movement but will still stay active

Discussion:

* Klaus gives a short preview for the discussion on Friday (Operando4NeXus)
* Property datatypes (#30)

  * mostly a bug in frappy
  * in general, custom properties can decide on their own datatypes (even
    arbitrary json, although maybe not recommended), and it is hard to specify.
  * can be closed

* Issue #6 can be closed, as it has been resolved (vocabulary for meaning extended)
    Klaus points out that more interaction with the NIAC is needed to convince them that we have
    a problem which needs a solution. (which may be the one we worked on).
    Since FairMat seems to be very interested on this topic, a sufficient momentum may be gained
    (summoning also people from Rock-IT, NF4DI, Daphne and so on).
    Biggest issue seems to introduce the needed flexibility into the static NeXus System.

There are currently no open RFC's


Interlude: New RFC's
--------------------

Niklas made two RFC's for discussion and presents them:

Addition of New Qualifiers for Protocol Extensions
    This RFC proposes extending the qualifiers for better integration with
    other protocols. One would be the post_change qualifier for the mqtt
    integration to transport the message type. The other would be the timestamp
    since the last write.

For the first qualifier most are fine if its MQTT specific
There is positive resonance on the timestamp for the last change of the target,
but some disagreement about specifics (when the change command came in, writing
to the hardware, when it was acknowledged by the hardware?)

On further discussion, the problem is largely that the message type from mqtt
is an update and not the changed like specified in SECoP. This can be solved
with a custom qualifier for mqtt-backed clients. Niklas reports that this also
represents the current state in octopy.

As a side point, it has to be checked that custom qualifiers are specified to
be starting with an underscore.

Protocol-Specific Mappings for Accessibles
    a protocol_mappings section with dictionaries for supported protocols and
    suggested mappings for the client when transporting over that protocol.

The mappings property causes some discussion because its meaning implementation
specific and the clients can do something else that specified. There
is some confusion on the direction of the translation. In the ESS case it will
be both directions with EPICS and SECoP. The use case is mainly for tracing and
debugging.

Georg proposes an optional history/tracing list/parameter to solve Niklas' problem.

There is a generally positive sentiment for something like this (also with
proxying secnodes).

Some proposed names: trace, history, trail, path, source, via, breadcrumbs,
itinary, data_route.

Niklas will modify his RFC's with the discussion and submit them to GitHub.

Discussion: RFC process impressions
-----------------------------------

Klaus finds it a bit harder to check the open RFC's in contrast to the old list
of issues.

There is no outright negative sentiment, in general it works and we will monitor
it.

Peter notes it is good to have a written discussion.

PS: We should all check our notification settings :)


Discussion: SECoP version 2
---------------------------

Do we need a version 2 of SECoP?

There were two changes that make it hard for older clients to connect to new
SEC nodes: meaning and visibility.

There are a few options:

* Make a clean break (2.0 version) now and have a stricter process afterwards
* small bump (1.1) and require clients to support both versions

There should be a note to update to the clients to keep them compatible.
Servers that are compliant to one version should not stop being that by a
change we make.

A note is made that well-written clients should degrade gracefully, but this
can't be a requirement.

Klaus argues that for the case that we have now we should do a major version
bump.

From a discussion between Georg and Markus: Laxer clients that don't check the
version but deal with both variants are still fine.

A rule should be that clients losing functionality, this should be a major
change, if clients just miss out on functionality then its a minor change.

Alexander asks if the backwards-compatible changes should to be collected into
a 1.1 version?

Conclusion:
    The version will have to be 2.0. It will not be released immediately. First
    we will check the personal to-do-lists and issues to check if there needs
    to be any other incompatible changes that could be swept into the major
    bump. The compatible changes will be reified into a version 1.1 and
    released.

Thorsten Bögershausen has to leave.


The meeting is closed at 18:00.

Compatibility Policy
--------------------

(from the agenda)

see the discussion above, in summary, clients should not lose functionality
between minor versions.
