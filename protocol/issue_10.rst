SECoP Issue 10: Character set for Names (under discussion)
==========================================================

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

Discussion
~~~~~~~~~~

Markus:
    For me, it is not that important, if uppercase characters are allowed or not, but,
    identifiers must be unique independent of case, i.e. it is not allowed to have two
    different modules "t" and "T", on a SEC node. And we should take a decision soon.

Default parameters 'value' and 'target'
---------------------------------------

In the implementation, in the "read" and "update" messages "<module>:value" can be just
replaced by "<module>", and in "change" and "changed" message "<module>:target" can
be replaced by "<module>".

Discussion
~~~~~~~~~~

Markus:
    I prefer not to allow these shortcuts. We also have to distinguish module properties
    from properties of the module parameter 'value'.




