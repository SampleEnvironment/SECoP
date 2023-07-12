- Feature: SECoP Requests for Comments
- Status: Open
- Submit Date: 2023-11-13
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Meta/Protocol

Summary
=======

This document describes the SECoP RFC process that is intended to replace the
previous SECoP issues and enable easier contributions from outside the SECoP
core team.

The goal is to have a standard path through which larger changes are introduced
into the specification of SECoP.  Bugfixes, reformatting, and changes of a
similar scope, which do not change the semantics of the protocol, do not have to
follow this process.

The RFC process orients itself after proven similar processes, for example in
the Rust and Python communities.  It is intended to be lightweight, but still
provide a frame to handle suggestions in a structured manner.


The Process
===========

Here we will lay out the steps to follow to change the specification.

- If you do not have a concrete idea, come talk to us to flesh it out.
- Copy the RFC-000-template.rst file and fill in the details. You can add
  sections freely, but try to fill out all the provided ones.
- Take the next free number and add it to the template file.
- The RFC gets discussed - this should mostly happen under the RFC pull request.
  If not, please add a summary if possible.  Optimally, an implementation will
  already be done at this stage in one or more of the SECoP implementations in
  order to uncover possible problems.
- There may be irreconcilable problems with the RFC. In that case, the committee
  can decide to reject the RFC, and it may not be reopened.  If there are new
  developments, or a new approach, a new RFC should be opened.
- If the RFC is in a satisfactory state, any of the SECoP committee members can
  propose to move to the finalization period.  This is a limited time of one
  month.  The RFC is broadcast on the appropriate channels to notify people of
  the last chance to comment.  If there are blocking issues uncovered in this
  period, it moves back to being under development.  If no major issues come up
  during the month, the RFC is accepted.


If you never used GitHub/Need Help
==================================

The process involving GitHub is mostly intended for the core team, if you e.g.
don't know how to use it, but have an awesome idea for SECoP, just make your
proposal using the template and contact us at isse@sampleenvironment.org.  One
of the core team will get back to you to help you with this process.
