SECoP Issue 12: JSON values vs documents (closed)
=================================================

Lets first define:

* a *JSON document* is either a *JSON object* or a *JSON array*
* a *simple JSON value* (for example a bare string or number) is a *JSON value*, which is not a *JSON document*

Some JSON libraries do not allow simple JSON values in their conversion functions.
Whether or not a simple JSON value is a valid JSON text, is controversial,
see this `stackoverflow issue <https://stackoverflow.com/questions/19569221>`_ and :rfc:`8259`

Decision
--------
The third part of a SECoP message can be any JSON value.

Programming Hint
----------------
If an implementation uses a libray, which can not convert simple JSON values,
the implemetation can add angular brackets around a JSON value, decode it
and take the first element of the result. When encoding the reverse action might be
used as a workaround. see also :RFC:`7493`

