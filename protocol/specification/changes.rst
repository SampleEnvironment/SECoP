Changes between versions
========================

This document tracks the various additions and breaking changes made in the
different versions of the SECoP specification.

Minor version updates like 1.0 → 1.1 only contain backwards compatible changes,
while major version updates can contain breaking changes.

A SEC node typically only implements one version of the specification.  To
maximize compatibility, all clients should strive to support all existing major
versions.


Version 2.0
-----------

This version is in the process of being finalized.

.. rubric:: Breaking changes

These changes make a 2.0 client necessary to connect to a 2.0 node without major
loss of functionality.

- The format of the `*IDN?` reply has been slightly changed and the meaning of
  the third field is now clarified.

- The `visibility` property for modules and accessibles has a new format with
  extended possibilities (:issue:`079 hints for readonly access`).

- The `meaning` property for modules has a different format with capabilities to
  link to external definitions/ontologies.  `meaning` can now also be given for
  individual accessibles, and new "functions" have been added.

.. TODO not yet in the text!

    - All :ref:`Struct <struct>` members can now be made optional by specifying
      ``"optional": true`` in the datainfo (:issue:`069 optional structs by
      default`).

.. rubric:: Backwards compatible changes

.. TODO machine readable, systems

- The specification text has been generally restructured and edited to improve
  clarity and presentation.

- The optional `check`/`checked` messages have been added, as well as the
  `checkable` accessible property (:issue:`075 New messages check and checked`).

- The :ref:`matrix <matrix>` data type has been specified for datainfo.

.. TODO

    - The `AcquisitionController`, `AcquisitionChannel` and `Acquisition` interface
      classes have been specified (:ref:`rfc-006`).

- The UDP discovery protocol has been specified (:ref:`rfc-005`).

- Optional SECoP transport over WebSockets has been specified (:ref:`rfc-007`).


Version 1.1
-----------

This minor revision was released in 2025 and is `available in the repository
<https://github.com/SampleEnvironment/SECoP/blob/master/protocol/SECoP_Specification_V1.1.rst>`_.

- The `~mod.implementation` property of modules has been added (:issue:`061 new
  predefined property implementation`).

- The `controlled_by` and `control_active` optional parameters have been
  specified, as well as the `control_off` command (:issue:`065 handling of
  coupled modules`).

- Features have been defined as a type of base class, and the `~mod.features`
  module property has been added (:issue:`072 features`).

- The `target_limits` parameter has been specified (:issue:`073 HasLimits and
  HasOffset`).

- The `HasOffset` feature and the `offset` parameter have been specified
  (:issue:`073 HasLimits and HasOffset`).


Version 1.0
-----------

This is the first published version of the specification released in 2019.  It
is available `in the repository
<https://github.com/SampleEnvironment/SECoP/blob/master/protocol/SECoP_Specification_V1.0.rst>`_.

Various pre-release candidates have been preserved `as well
<https://github.com/SampleEnvironment/SECoP/tree/master/protocol/candidates>`_.
