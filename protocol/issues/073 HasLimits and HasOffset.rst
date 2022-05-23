SECoP Issue 73: HasLimits and HasOffset (proposed)
==================================================

Motivation
----------

A way to define user settable limits for valid targets and a offset adjustment are
typically used in many ECS. Defining this in a standardised way will be beneficial.


Proposal
--------

predefined parameter "offset"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The offset parameter contains a value with the purpose to correct for a nearly
linear error of the main value. Typically the following applies:

   physical value = raw value + offset

In some cases, the correction formal might differ, however, it is guaranteed, that
the same offset always leads to the same correction.


predefined command  "adjust"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The adjust command corrects a given old value to a corrected new value.
It has a struct argument with the following datainfo:

{"type": "struct", "members": {"old": <numeric>, "new": <numeric>, optional=["old"]}}

When this command is called the offset correction is done in a way, that the
old value given is corresponding to the new value after the correction.
If the old member is not given, the current main value is used for it.

<numeric> is typically of type "double" or "scaled", but it might also be structured in
the same way as the main value, e.g. as a tuple of "double".


predefined parameter "limits"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

"limits" is structured as a tuple with two numeric members indicating the lower and
upper end of a valid interval for the setting the target parameter.

The SEC node must raise an error in case a given target value does not fit into the interval.
It is recommended that an offset correction updates also the current limits.


predefined property "abslimits"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The "abslimits" property, is, if present, a JSON array with two numeric values,
containing the uncorrected, raw limits. This property is recommended, when there
is an offset parameter which follows the following formula:

   physical value = raw value + offset

If the "abslimits" property is not present, the ECS typically looks at the datainfo of
the target parameter for the valid range.


feature HasOffset
~~~~~~~~~~~~~~~~~

A module with this feature has an "offset" parameter and optionally an "adjust" command


feature HasLimits
~~~~~~~~~~~~~~~~~

A module with this feature has an "limits" parameter, and optionally an "abslimits"
property.
It must not have an "abslimits" property, if the "offset" parameter does not follow the
standard formula "physical value = raw value + offset"


feature HasCustomLimits
~~~~~~~~~~~~~~~~~~~~~~~

It would be nice for the ECS to know, if the standard formula is used for
the correction or not. To cover this we might offer two flavors:

* "HasOffset" must use the standard formula "physical value = raw value + offset",
  the "adjust" command is optional, as it may be done by the ECS
* "HasCustomOffset" does not use the formula above, the "adjust" command is mandatory.
  A module with the feature "HasCustomOffset" must then NOT have the "abslimits" property.



