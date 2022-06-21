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
linear error of the main value. Typically the following applies:

   physical value = raw value + offset

In some cases, the correction formula might differ, however, it is guaranteed, that
the same offset always leads to the same correction.


predefined command  ``adjust``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The adjust command corrects a given old value to a corrected new value.
It has a struct argument with the following datainfo:

{"type": "struct", "members": {"old": <numeric>, "new": <numeric>, optional=["old"]}}

When this command is called the offset correction is done in a way, that the
old value given is corresponding to the new value after the correction.
If the ``old`` member is not given, the current main value is used for it.

<numeric> is typically of type ``double`` or ``scaled``, but it might also be structured in
the same way as the main value, e.g. as a tuple of ``double``.


predefined parameter ``limits``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``limits`` is structured as a tuple with two numeric members indicating the lower and
upper end of a valid interval for the setting the target parameter.

The SEC node must raise an error in case a given target value does not fit into the interval.
It is recommended that an offset correction updates also the current limits.

Alternative names: userlimits, softlimits


predefined parameter ``hardlimits``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``hardlimits`` parameter is a tuple of two numeric members indicating the lower and
upper end of a valid interval for the values of the ``limits`` parameter.
If there is no ``offset`` parameter, ``hardlimits`` is not needed, as the datainfo
range of the ``target`` gives this information.
``hardlimits`` might also be omitted in case the ``target`` range is infinite, for example
with an endless rotating stick motor.


predefined property ``abslimits``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``abslimits`` parameter is a tuple of two numeric members indicating the lower and
upper end of uncorrected hardware values. This is useful only in case when there is an
``offset`` using the standard formula.


feature HasOffset
~~~~~~~~~~~~~~~~~

A module with this feature has an ``offset`` parameter and an ``adjust`` command.


feature HasStandardOffset
~~~~~~~~~~~~~~~~~~~~~~~~~

A module with this feature has an ``offset`` parameter using the standard
formula ``physical value = raw value + offset``. The adjust command is optional,
as it may be done on the ECS. A module can not have both features HasOffset and
HasStandardOffset.
The property 'abslimits' is also optional.

Remark: this is a case, where a feature is needed in addition to the predefined
parameter, as the semantics of the ``offset`` parameter is narrowed.


feature HasLimits
~~~~~~~~~~~~~~~~~

A module with this feature has a ``limits`` parameter, and optionally a
``hardlimits`` parameter.


necessity of features over predefined parameters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

It is not strictly needed to specify the features HasOffset and HasLimits,
as the meaning of the predefined parameters are sufficient. The feature
HasStandardOffset however narrows the semantics of the ``offset`` parameter, so the
presence of the ``offset`` parameter alone is not sufficient to detect this behaviour.


Discussion
----------

As fixed limits are already given in the datainfo, the interplay with user changeable
limits has to be considered.

At the meeting on 2022-06-01, a discussion about the exact meaning of the ``min`` and
``max`` datainfo properties raise up. First we have to agree about the validity of
the following statements:


Statements
~~~~~~~~~~

1) A SEC node programmer should do its best to give ``min`` and ``max`` in a way, that
   writable parameters accept all values within the range without error.

2) The SEC node can change (datainfo) properties only by closing the connection.
   The client may receive changed properties from the description after connecting again.

3) Changing properties (and therefore closing connections) should be avoided.

4) If, for any reason, the allowed range of a parameter may change over time, the
   ``min`` and  ``max`` datainfo properties should be omitted, in order to indicate
   to the ECS, that limits may change.

5) In case the changed range applies to the ``target`` parameter, the limits may be
   given by a predefined user limits parameter.

6) Statement (3) has priority over (1) in the case of a user changeable offset: i.e.
   we should not change datainfo and force a reconnect in case the offset is changed.


Questions
~~~~~~~~~

7)  A vector magnet, where the main value is a vector implemented as a tuple,
    may be limited by the absolute value of the vector. Say the individual components
    are limited to [-1, 1], and the of the absolute value of the vector is limited to 1.
    A valid target would be [1, 0, 0] but not [0.8, 0.8, 0.8].
    According to (4), it should not give ``min`` and ``max`` for its members datainfo
    properties. Would it not be better to set ``min`` and ``max`` to the maximum
    allowed individual values? If this is true, we should relax (4) to:

        If, for any reason, the allowed range of a parameter may change over time, the
        ``min`` and  ``max`` datainfo properties should be set to the minimum and maximum
        possible values, for example adding the ``offset`` range to the ``target`` range.

8)  For another parameter than ``target``, when its limits may vary, is it better to
    indicate no limits, or lower/upper bounds of the limits?
