SECoP Issue 73: HasLimits and HasOffset (proposed)
==================================================

Motivation
----------

A way to define user settable limits for valid targets and a offset adjustment are
typically used in many ECS. Defining this in a standardised way will be beneficial.


Proposal
--------

predefined parameter ``offset``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The offset parameter contains a value with the purpose to correct for a nearly
linear error of the main value. The following applies:

   physical value = raw value + offset



predefined parameter ``target_limits``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``target_limits`` is structured as a tuple with two numeric members indicating
the lower and upper end of a valid interval for the setting the target
parameter.

The SEC node must raise an error in case a given target value does not fit
into the interval. It is recommended that an offset correction updates also
the current limits.

The name is choosen in a way for a possible extension to have dynamic limits
an any parameter, in case the use case would pop up.


in the initial proposal, above parameters were part of proposed features
'HasOffset' and 'HasLimits'. However, as already with other predefined
parameters, and as only one parameter is involved with a propsed features,
the definition of predefined parameters is sufficient.


Discussion
----------

Basics to be discussed before defining combination of offset and limits
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As fixed limits are already given in the datainfo, the interplay with user
changeable limits has to be considered.

At the meeting on 2022-06-01, a discussion about the exact meaning of the
``min`` and ``max`` datainfo properties raise up.

Finally, we agree that is it acceptable, that changing the target parameter
might raise an error, even if the value is inside the specified range
in datainfo. This would be the case when the target parameter is set to a value
outside target_limits, but inside the targets datainfo limits.
Also changing the target_limits might raise an error when the values are
outside internal absolute limits, which are typically configured before
offset correction. 
