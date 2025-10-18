SECoP Issue 10: Character set for Names (closed)
================================================

Uppercase Characters in Identifiers
-----------------------------------

Actually (V2017-09-14), the following is specified;

The identifiers are composed by
lowercase ascii letters, digits and underscore, where a digit may not
appear as the first character. Identifiers starting with underscore are
reserved for special purposes like internal use for debugging. The
identifier length is limited (<=63 characters).

In the outdated part of the documentation (`outdated Messages`_) it is not
explicitly stated, that identifiers have to be lower case, and also
in some of the running examples contain uppercase identifiers.

.. _`outdated Messages`: messages.html

Decision
~~~~~~~~

Names may contain uppercased letters as long as the lowercase version is still unique.





