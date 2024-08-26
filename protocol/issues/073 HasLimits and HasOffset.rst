SECoP Issue 73: HasLimits and HasOffset (proposed)
==================================================

Motivation
----------

A way to define user settable limits for valid targets and a offset adjustment are
typically used in many ECS. Defining this in a standardised way will be beneficial.


Proposal
--------

feature ``HasOffset``
~~~~~~~~~~~~~~~~~~~~~

This feature is indicating, that the value and target parameters are raw values, which might need to
be corrected by an offset. A module with the feature HasOffset must (or may?) have a parameter offset,
which indicates to all clients, that the logical value may be obtained by the following formula

  logical value = raw value + offset


predefined parameter ``target_limits``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``target_limits`` is structured as a tuple with two numeric members indicating
the lower and upper end of a valid interval for the setting the target
parameter.

The SEC node must raise an error in case a given target value does not fit
into the interval. 

The name is choosen in a way for a possible extension to have dynamic limits
an any parameter, in case the use case would pop up.


Discussion
----------

A member of the committee dislike that the limits in the datainfo, and therefore the
descriptive data might be changed by a client, needing a rebuild of the structures on
the ECS side. An other member of the committee does not agree that the datainfo min and max of the
target_limits parameter have to be omitted or set to a bigger range than the currently valid ones.

As a consequence, if we try to respect these two positions, SECoP must always handle raw values,
and the offset correction has to be shifted to the client side.


specification of the offset parameters datatype
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The offset is a floating point number ('double' or possbily 'scaled'), and when
there is an offset, the value must also be a floating point number. This is
quite evident, and it is not needed to put this into the specs, because a
precise wording is diffcult and lengthy.

* is it allowed that 'offset' is 'double' and 'value' is 'scaled' ?
* units should match, but from physics, when value has unit Celsius, 'offset' must have unit Kelvin
* min, max do not need to match
* are structured types allowed?
* string, enum, bool, blob: not allowed as data type
* is int allowed ?

An implementor naturally should choose a reasonable datatype, and we do not need to
clarify this in detail.


Current State
-------------

HasOffset: finalized

HasLimits (or how to handle limits at all): under discussion or at least not yet finalized.
On the meeting 2023-09-26 it was decided to use postfixes '_min' and '_max' for changeable limits of parameters.
