- Feature: SECoP Requests for Comments
- Status: Accepted
- Submit Date: 2023-11-13
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Meta
- Version: none

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


Lifecycle of an RFC
===================

1. An RFC starts as a pull request against the SECoP GitHub repository.

2. First discussion happens in the form of comments on that pull request, and/or
   in committee meetings.

   It is encouraged to start adding the new feature to SECoP implementations to
   collect real-life experience and uncover unexpected problems.

3. As soon as it is agreed that the idea has merit and a chance to be accepted,
   it gets assigned an RFC number and the pull request is merged.

4. The RFC stays in state "Open" and can be further discussed.  Real-life test
   implementation continues.

   It is encouraged to start early on updating the specification to include
   changes from the RFC.  The link to the pull request is added to the "PR"
   header.

5. When the committee feels the RFC is in a satisfactory state, it moves the RFC
   to the "Accepted" state.  At this point, the pull request to the
   specification must be done.  This starts a finalization period of usually a
   month.

6. If no further objections are noted during this period, the RFC is moved to
   the "Final" state, otherwise back to "Open".

7. Alternately, if the committee at any point feels the RFC is not viable, it is
   moved to the "Rejected" state.


How to contribute
=================

Here we lay out the steps to follow to change the specification.

- If you do not have a concrete idea, come talk to us to flesh it out.
- Copy the RFC-template.rst file and fill in the details. You can add
  sections freely, but try to fill out all the provided ones.
- Take the next free number and add it to the template file.  Renumbering might
  happen when the PR is merged.
- The RFC gets discussed - this should mostly happen under the RFC pull request.
  If not, please add a summary if possible.  Optimally, an implementation will
  already be done at this stage in one or more of the SECoP implementations in
  order to uncover possible problems.
- If the proposal is sound enough and has general approval, the RFC PR is merged.
  Further discussion can and should of course happen.
- There may be irreconcilable problems with the RFC. In that case, the committee
  can decide to reject the RFC, and it may not be reopened.  If there are new
  developments, or a new approach, a new RFC should be opened.
- If the RFC is in a satisfactory state, any of the SECoP committee members can
  propose to move to the finalization period.  This is a limited time of one
  month.  The RFC is broadcast on the appropriate channels to notify people of
  the last chance to comment.  If there are blocking issues uncovered in this
  period, it moves back to Open.  If no major issues come up during the month,
  the RFC is made Final.


If you never used GitHub/Need Help
==================================

The process involving GitHub is mostly intended for the core team, if you e.g.
don't know how to use it, but have an awesome idea for SECoP, just make your
proposal using the template and contact us at isse@sampleenvironment.org.  One
of the core team will get back to you to help you with this process.
