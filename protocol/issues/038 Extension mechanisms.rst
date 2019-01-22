SECoP Issue 38: Extension mechanisms (under discussion)
=======================================================

Motivation
-----------
In the current specification the extension mechanisms are not clarified.
This should be rectified with this Issue.

Proposal
--------

Add the following list of extension mechanisms to the specification:

* add actions, keeping the 'triple' structure of action/specifier/data

  *note:* Thats why custom actions MUST be prefixed with an underscore.

* extent specifier with ':' separated identifiers, getting more and more specific

  *note:* this may also be used to more precisely specifiy errors, as the errorclass is transferred in the specifier field.
  i.e. the ``BadValue`` errorclass may be extended into ``BadValue:WrongType``, ``BadValue:IllegalValue``, etc.... subclasses.

* define additional parameter/command/property names

* extend reports (only append to them, never touch the already defined fields)

  *note:* The structure report may need to be nested inside a json-array in the future, should we need to extend that.

* use so far unused datafields (there are not so many).

* define additional status groups or statuscodes

* define additional interface classes/features


Discussion
----------

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Decisions:
 - Should be explained in more detail, as it is intended to give hints to implementors.
 - keep it under discussion for now.
