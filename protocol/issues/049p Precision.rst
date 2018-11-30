SECoP Issue 49: Precision of Floating Point Values
==================================================

Motivation
----------

For the client it might be useful to know about the precision of a value.
This might be useful for UI purposes (how many digits should ne shown),
and for other reasons (what is the tolerance to consider two values as the same).

Proposal
--------

in the vidconf 2018-11-07 it was proposed to make a new property
"precision" beeing a tuple (<absolute precision>, <number of significant digits>).

Solution a: fmtstr only
+++++++++++++++++++++++

For the UI, the easiest would be to specify a C-style format %.<digits>(e|f|g)
as name Enno proposes "fmtstr". Markus would strongly advise to be strict on the
syntax: do not allow other characters in the format, no field size etc.

A SEC node specifying a "fmtstr" would tell the client, that it makes no sense to
use more digits than given in the fmtstr, as the hidden digits have no significance.

Solution b: absprecision and relprecision
+++++++++++++++++++++++++++++++++++++++++

For other purposes, and to allow more precision on the precision, we may spcifiy a
"step" size as proposed above, but Markus would prefer to split into two
properties "absprecision" and "relprecision", both beeing a floating point value,
each of them is optional. If both are present, then the precision is 
max(absprecision, abs(value) * relprecision).

Solution c: both a and b
++++++++++++++++++++++++

Solution d: fmtstr and absprecision
+++++++++++++++++++++++++++++++++++

"absprecision" could then be named "precision"


