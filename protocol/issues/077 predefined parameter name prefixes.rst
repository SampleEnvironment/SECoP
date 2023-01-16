SECoP Issue 77: predefined parameter name prefixes (proposed)
=============================================================

Motivation
----------

* Sometimes the valid range of a parameter ``<param>`` needs to by dynamically changed
  during runtime. Typically adding a ``max_<param>``, ``min_<param>`` or
  ``<param>_limits`` parameter is discussed in those cases.
* There are also cases, where the effect of a parameter needs to be
  'activated'/'enabled' or 'disabled' during runtime without overwriting
  the actual value of parameter. Also using 'magic' values for disabling
  purposes is bad practice.
* There may be more use cases, where *dynamic* parameter properties would be
  helpful.

To avoid restarting the, essentially, same discussion over and over again,
a generic way of using appropriate named parameters instead of dynamic
parameter properties.


Proposal
--------

It is proposed to add the following to the names of parameters section of the spec:

:note: All names are provisional subject to finding better ones.

:note: All prefixed extra parameters are optional.

All predefined parameters may be accompanied by one ore more of the following,
prefixed parameters which are used instead of changeable parameter properties.

:note: In the below listing, ``<paramname>`` is a placeholder for the name of the relevant parameter.

``use_<paramname>``
  an Enum (1:On, 0:Off), allowing to enable or disable the effect of
  ``<paramname>`` where applicable.

``limit_<paramname>``
  a Tuple(minvalue, maxvalue) specifying the changeable limits of parameter
  ``<paramname>``.

``min_<paramname>``
  a value (of typically the same datatype as ``<paramname>``), specifying the
  changeable minimum accepted value of a parameter ``<paramname>``.

``max_<paramname>``
  a value (of typically the same datatype as ``<paramname>``), specifying the
  changeable maximum accepted value of a parameter ``<paramname>``.

:note: ``limit_<paramname>``, ``max_<paramname>`` and ``min_<paramname>``
  exclude each other, i.e. may not coexist at the same time at the same module.


Discussion
----------

In the discussion at the meeting from 2023-01-16 we found that it is probably better
to use a postfix, as this would take related parameters together when sorted
alphabetically.

Possible candidates:

* 'ramp_used' or 'ramp_enable' instead of 'use_ramp'
* 'current_max' or 'current_limit' instead of 'max_current'
* keep 'target_limits' (with 's' at end in contrast to '_limit')
* 'target_max' / 'target_min' in case of single limits

