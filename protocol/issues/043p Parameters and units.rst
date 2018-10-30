SECoP Issue 43: Parameters and units
====================================

The problem
-----------
This Issue is about two related topics:
 a) Modules may have several parameters which somehow refer to the unit of the main value.
    That can be a ``ramp`` of a temperature or magnetic field, ``speed`` of a motor, or several others.
    In both cases, the unit of the parameter has an implied reference to the unit of the main value.
    This is no problem in the static case, but as soon as units become variable, it is very important to handle this right.
    It has implications from Issue31 also, which could then be at least detected.
 b) it is so far undefined how the unit of a structured parameter is to be given/handled.
    Also, it is so far unclear if and how the unit of a argument or result to/from a command can be described.

Proposal: a)
------------

One way to make the dependence of the unit more clear is by not 'copying' the unit of the main value, but by
using a placeholder instead. So a ``ramp`` could have a unit of "main/min", ``speed`` could have a unit of "main/s"
and an accelleration a unit of "main/s^2". In these examples, "main" was used as the placeholder.
"$" could be used as well as other short strings.
Using "main" has the benefit of referring to something existing (the unit of the main value), so a
possible extension could also be to use the name of an parameter of the module to represent 'the unit of that parameter'.

The most important rules for this are:
 - we get a reference to the unit of another parameter (don't build loops!)
 - this reference can not be confused with a regular unit.

Proposal: b)
------------
Lets distinguish between unitless datatypes (bool, enum, string, blob, command), simple datatypes (double, int)
and structured datatypes (tuple, array, struct, command).
For composing the unit of a parameter with a complex datatype the following translations should be made:

derivation of unit-structure:
 1) unitless datatypes become a unit of ``null`` assigned.
 2) simple datatypes become a unit describing the physical unit as a string.
 3) structured datatypes keep their structure which contains the units of the elements of the structure:
    - a tuple copies the units for each subelement, i.e. result in a tuple of strings
    - an array has the unit of the subelement, prepended by '*', i.e. result in one string
    - a struct maps the names of the subelements to their units.
    - a command results in a tuple with the units of the argument and the result.

After this, all parts containing a ``null`` are removed.

These rules effectively do not change the unit for simple datatypes (which are the majority).

An example may help:

Lets assume we have a parameter which is an tuple of an enum, a string, an array of a double and a struct of an bool, a blob and an int.
the datatype of that parameter could then look like::

 ["tuple", [ ["enum",{"a":1,"b":2,...}],
             ["string",20],
             ["array", ["double"], 2, 5],             ; unit is "V"
             ["struct", {"c":["bool"],
                         "d":["blob",10],
                         "e":["int"]}] ]]             ; unit is "A"

(structurally indented for better readability.)

According to the rules, this would be the intermediary result before removing all ``null``'s::

 [                    ;the unit structure of a tuple is a tuple
  null,               ;the enum has no unit
  null,               ;string has no unit
  "*V",               ;unit of the array entries
  {"c":null, "d":null, e:"A"}  ;only the unit of the int is not null
 ]


Finally, the ``unit`` property of that fictive parameter would then be: ``["*V",{"e":"A"}]"``, which is no longer a string.

An ECS not able to handle non-string units (like the one in this example) should not display a unit at all, or give some indication
to the user that unit is more complex. Otherwise these structured units are a great hint to UI's as they can now
display the correct unit for subelements.


Discussion
----------
never discussed. So far the unit property was thought, to always be a string.
