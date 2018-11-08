Default parameters 'value' and 'target' (closed)
================================================

In some implementations, in the "read" and "update" messages "<module>:value" can be just
replaced by "<module>", and in "change" and "changed" message "<module>:target" can
be replaced by "<module>".

Decision
--------

All messages sent by clients and servers MUST contain the parameter name when referring
to the *value* or *target* parameter. However, they may support these shortcuts on
messages received, but they do not need to.
