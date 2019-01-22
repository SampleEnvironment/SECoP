SECoP Issue 32: Clarify specification using RFC-style definitions (closed)
==========================================================================

The problem
-----------
Any successfull specification so far had to cope with different ways of reading them.
Different opinions and points of view of people trying to implement any given
specification will often result in incompatible or unexpected behaviour,
if the specification is not precise enough.

As we want SECoP to interoperate equipment made by a broad range of institutions and companies,
we therefore need to make it as clear as possible, what is allowed and where
there is implementation freedom.

Looking at how other institutions managed this (successfully), one stumbles over
the internet protocols.
The are written by one institution as rfc and implemented by many people,
still those implementations are largely interoperable.
This is achieved by following a *very* *formal* description of the protocol and a
specified review process.

Where needed (and possible with our knowledge) we should therefore also provide a
rfc-inspired definition of SECoP.

Currently the serialisation of messages seems to be an easy enough target to rewrite
that section using :RFC:`5234`.

Proposal
--------
Enrico proposes to include references to relevant rfc's for some definitions
(and to also use the therein defined wording).

utf-8
  as defined in :RFC:`3629`

base64
  as defined in :RFC:`4648`

JSON
  as defined in :RFC:`8259` + interoperability hints in :RFC:`7493`

key words
  as defined in :RFC:`2119` + clarified in :RFC:`8174`

syntax ABNF
  as defined in :RFC:`5234`

The section about the (de-)serialisation of messages should be reworded using :RFC:`5234`.
(see also `Issue 30: Clarify message parsing`_)

.. _`Issue 30: Clarify message parsing`: 030p%20Clarify%20message%20parsing.rst



Discussion
----------

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Decisions:
 - proposal accepted, i.e. the current sigle file specification will get reworked
 - a later specification should be in rfc style with two documents: 'basic' and 'extensions'
 - issue is to be closed for now.


