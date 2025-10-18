- Feature: Definition of the term 'Qualifier'
- Status: Final
- Submit Date: 2017-05-30
- Authors: SECoP committee
- Type: Issue
- PR:
- Version: 1.0

Summary
=======

Specifies qualifiers as the "properties" of live data.


Issue text
==========

The definition as in SECoP V2016-11-30 draft is not very consistent.

As decriptive data we have:

- SEC node properties
- module properties
- parameters properties
- command properties

Live data may be:
- value
- timestamp 't'
- sigma 'e'

It is confusing to use the same term 'property' for live data and for
descriptive data.

The section with the definition of properties has to be rewritten.

Opinions
--------

Enrico proposed to name live data like timestamps and sigma *qualifiers*.

Markus supports this. He would be happy also with a term other than
*qualifiers*, but definitely does not like the terms *live properties* and
*descriptive properties*, as two different things share the same name.


Decision
--------

Additional information transported with an update message like timestamps and
sigma are called *qualifiers*.
