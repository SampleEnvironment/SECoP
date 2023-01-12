SECoP Issue 77: predefined parameter name prefixes (proposed)
=============================================================

Motivation
----------

* Sometimes the valid range of a parameter ``<param>`` needs to by dynmically changed
  during runtime. typically adding a ``max_<param>``, ``min_<param>`` or
  ``<param>_limits`` parameter is duscussed in those cases.
* There are also cases, where the effect of a parameter needs to be
  'activated'/'enabled' or 'disabled' during runtime without overwriting
  the actual value of parameter. Also using 'magic' values for disabling
  purposes is bad practice.
* there may be more use cases, where *dynamic* parameter properties would be
  helpful.

To avoid restarting the, essentially, same discussion over and over again,
a generic way of using appropriate named parameters instead of dynamic
parameter properties.


Proposal
--------

It is proposed to the following to the names of parameters section of the spec:

:note: All names are provisional subject to finding better ones.

:note: All prefixed extra parameters are optional.

All predefined parameters may be accompanied by one ore more of the following,
prefixed parameters which are used instead of changeable parameter properties.

:note: In the below listing, ``<paramname>`` is a placeholder for the name of the relevant parameter.

``use_<paramname>``
  an Enum (1:On, 0:Off), allowing to enable or disbale the effect of
  ``<paramname>`` where applicaple

``limit_<paramname>``
  an tuple(minvalue, maxvalue) specifying the changeable limits of parameter
  <paramname>.

``min_<paramname>``
  an value (of typically the same datatype as ``<paramname>``, specifying the
  changeable minimum accepted value of a parameter <paramname>.

``max_<paramname>``
  an value (of typically the same datatype as ``<paramname>``, specifying the
  changeable maximum accepted value of a parameter <paramname>.

:note: ``limit_<paramname>``, ``max_<paramname>`` and ``min_<paramname``
  exclude each other, i.e. may not coexist at the same time at the same module.

Discussion
----------

