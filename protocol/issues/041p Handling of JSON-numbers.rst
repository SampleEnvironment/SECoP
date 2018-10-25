SECoP Issue 41: Handling of JSON-numbers
========================================

Motivation
----------
A JSON-number can only be a numeric value, while the datatypes used in programs can
also hold 'values' like "inf" or "NaN".
At the moment the behaviour upon encountering such values is implementation specific.
Unfortunately it is very dependent on the JSON library if emitting or parsing such values work, or what happens.

Proposal
--------
We should define how to handle this. following ideas should be discussed:

1) forbid emitting non-numerical values. A request containing such a value MUST then be answered with an ``error``.
    if a reply contains a non-numeric value, the message is to be ignored by the ECS.
2) map such nun-numeric values to the JSON-value ``null``.
3) represent the non-numeric values as strings "+inf", "-inf", "+NaN", "-Nan".

in case 2) and 3) software may need to investigate the type in the JSON before converting to a number and check values before emitting a JSON.


Discussion
----------
Once discussed briefly, but neglected as not relevant.
Should still be defined instead of implementation specifc.

Enrico votes for 1), falling back to 3), if 1) finds no majority.
