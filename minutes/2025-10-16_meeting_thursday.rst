SECoP in person workshop at ESS - Thursday
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.. sidebar:: participants

     * Georg Brandl
     * Peter Braun
     * Niklas Eckström
     * Enrico Faulhaber (remote)
     * Klaus Kiefer
     * Alexander Zaft
     * Markus Zolliker

.. contents:: Agenda
    :local:
    :depth: 3




2) Documentation and Outreach
=============================

Outreach: how to best get  new people on board
----------------------------------------------

Klaus gives a short presentation.
There need to be easy access points for new people.
The spec has to be readable, something that we generally succeeded in.
Our implementations and tools should be up to date. Are there any tools missing?
Q&A section might be a good idea.
Shold we do some

Klaus asks, whether there is anything missing?

Specs
-----

There is some discussion if we are trying to do two things with our specs: the
specification itself and an easy introduction. Georg mentions that an index
(using custom rst structs for the spinx) should be made for messages and other
constructs.

Peter mentions that there are two types of people that need to read the spec,
those who write a client and those who write a secnode. The first of these
being much more involved. The second group may need a more focussed guide?

Markus mentions that choosing the right datatype is a good example of something
that should be in the documentation of the framework. Reading the specification
for the framework/client should include as much as possible so that it is not a
requirement to read the spec first necessarily.

Georg: there are two levels, syntax and semantics, where the syntax part is
normally done by the frameworks but you need the semantics if you want to
extend a module you would need to know the semantics. In the spec we currently
mix this.

Klaus would like to at least see an example of how it would look on the wire in
the semantics sections.

Niklas mentions that some feedback he got was that we do not have enough examples.

The structure should roughly be: Motivation and general operation / semantics /
syntax / example.

Georg and Klaus agree that if you have a good guide, you would not need e.g.
the introduction in the spec and it can be more concise.

In general the way it's written should be `example -> explanation`, not placing
the example at the end. Should the railroad diagrams be moved to the end or
made collapsible (closed default)?

As an aside, a pdf version would be nice to have.

Does everyone have secop linked in their SE sites?

Dunforce?

Web-Client
----------

There is no dedicated example clients yet.
How is it with websockets and cross-origin requests?
Is the restriction to not install anything or would a simple download be fine?

For a downloadable executable one would have to maintain versions at least for
the three different operating systems. Peter's tool is intended more for
someone to administer it and provide it to others in the facility.

Georg proposes a single python file, since people could run that however they
want. This would go around a lot of the packaging. For demo purposes it should
be small enough.

Klaus would still like a more generic client than just for the demo. Georg says
it could be made so that this client can be shrunk to the demo.

Examples
--------

Examples wiht full code to download for a demonstration?

Self-contained web-simulation with simulated devices.

Contact and  Help Channels
--------------------------

Klaus would like to have a forum for questions.

Possible options that are discussed:

    * email contacts
    * forum (github discussions)
    * email list (could get a lot of spam)

Whatever we choose, we should link to it on the website.

Community Events
----------------

Klaus: nice to see that you see secop popping up in many places.

It would be nice for community events online or in person to take place.

Klaus would like to set up a newsletter and establish it in time for the next
standardization in sample environment committee.

3) Spec, ctd.
=============

Machine readable spec
---------------------

Georg starts presenting the updated machine readable specification.

There is a question about needing the leading underscore on custom e.g.
parameters when there is a yaml specification.

The can be name clashes between two facility specific behaiours, but this is
secondary to the difference between experimental and accepted, which the
underscore is there to highlight.

RFC-002 introduces the schema of how to construct the yaml files.

* everything has a kind, name and version
* hierarchy and overriding is possible
* currently there is no way to enforce a specific value of a property, this
  could be extended later to make it checkable
* right now, its the latest version of the specs, for the release of 1.1
  and 2.0 these need to be split out

Klaus asks a clarifying question about the power supply system and the location
of the repository files.

RFC-003 specifies the method to link to the specs from the descriptive message.

* lists of urls to some files containing the spec

Markus asks whether the SECoP version of the node should then be duplicated
into the schemas dictionary or if it should be only in the identification
message. One possibility would be to have the checker verify that they are the same.

Peter mentions that the yaml files should be split out of the checker repo into
a separate repo.

Markus asks that the description in the yaml files could be left empty if its
not needed. It's fine since its optional right now.

Peter proposes an extra link field to link to the spec. We have to be careful
to have these links persistently.

Alexander shows the current state of the checker and the small web version.

It would be nice to have an extended verion hosted somewhere. Alexander will try
to do that at MLZ.

RFC-004 defines the systems field in the descriptive message, allowing for
multi-module complexes to be specified.

* the present systems are declared, and their component names are mapped to
  the actually existing module names.
* it can be nested

Klaus makes the point that you should not nest systems in the definition, but
declare them in the topmost level and then use their name in the other system.
-> No anonymous systems

Alexander notes that the subsystems are missing in RFC-002, Georg will fix that.

We agree that system names should not clash with module names and that we won't
allow "anonymous systems" and give each their name.

The point is made that we have to draw up a changelog between the different versions.
Georg will draw up the list of features for each version.

Smaller spec topics
-------------------

In the context of spec compliancy of the current implementations, there is a
discussion on the go command.
At least frappy has to be updated.
The question if the go command should buffer only when its running or always.
Markus proposes to change the target last instead, but this may not work since
the target may not even change.

There is an option to pull it out of the drivable with 2.0.
Would it make sense as a qualifier?

Conclusion:
    the functionality is needed at some point, the question is the kind of
    implementation. This has to be reevaluated at the next meeting. At least
    the documentation has to be clarified.

---- Lunch break ----

General parameter modifiers:

It was decided in 2023 that the predefined parameter x-fixes should be suffixes
and that we wanted to have it. An PR and a list of candidates have to be drawn
up. Peter already used this and Frappy implements most of this, so there are
existing examples already. Peter will do this.

There are already parameters with underscores, so we can't enforce that the
underscores are reserved for this mechanism.

4) Outreach work distribution
=============================

We breakout two sessions, one starts to work on some of the discussed spec
topics, the other with the website.

Group 1: Specification
    Want to move things into to the Overview section, shorter in the specific
    sections with examples still on top. Splitting off the parameters and
    commands into a section. Ident message review. The extra info should maybe
    indicate preview changes.

Group 2: Website
    * Fleshed out the list of guides.
    * Make the "Getting started" page a distribution site for users.
    * The work will be distributed in session 6.

5) More ways to integrating SECoP
=================================

Andreas Hagelberg joins.

Open discussion round, just noting key points:

Is there a SECoP to Tango bridge?
    Not yet, MLZ only has the other way around. Can be done though. Contacting
    Jan Kotianski may be a good idea to get information.

OPC-OA (done by SOLEIL)
    no one has concrete plans creating a mapping right now, but Niklas is
    interested in looking into it

Current situation at ESS
    * Plan for Octopy is to have a mode/option to have it slimmed down so that
      you can publish a secnode over MQTT. Can act as an IOC.
    * Octopy currently supports commands without arguments.
    * To differentiate the return to two commands, there are client ids in
      some cases.
    * Andreas shows the structure of octopy in MQTT Explorer
    * secop gateway has to handle disconnection
      in depth discussion about restrictions, naming_schemes and structures
      which are in experimantal use or are proposed for future implementations.

There is some discussion about transferring SECoP via other protocols like
EPICS and MQTT.

SECoP to NICOS at ESS
Niklas wants to do a workshop/sprint/similar to demonstrate the acquisition
directly from octopy through NICOS to Kafka.
He is in the process of gathering the requirements/benchmarks from ESS.
There is an open question of how NICOS's Kafkawriter works in respect to
dynamic devices.

Markus quickly shows how it looks at PSI. And the
Georg shows the general way to do a SECoP setup at MLZ.

Andreas has to leave.

Since Georg is leaving tomorrow morning, there is some preview discussion for
tomorrow. This includes Operando4NeXus and the website work.

18:00 End

--- Dinner ---
