- Feature: Linking to Ontologies
- Status: Open
- Submit Date: <your date>
- Authors: Alexander Zaft <a.zaft@fz-juelich.de>, Enrico Faulhaber
  <enrico.faulhaber@frm2.tum.de>, Georg Brandl <g.brandl@fz-juelich.de>
- Type: Protocol
- PR: <link to the pull-request>
- Implementation Tracking: -

Summary
=======

This Feature introduces a way to include links to definitions about protocol elements
in the descriptive data of a SECNode.
This is achieved by linking to the append-only ontology definitions introduced in
RFC-xxx.


Goal
====

SECNodes should link to the specifications for their protocol elements in order to
enable three things:

- The implementors of clients can get a description of the functionality of these interface classes.
- A client can use the extended description to provide further functionality.
- The structure of the SECNode can be verified to follow the interface.

Protocol elements are:

- Interface classes
- Features
- Accessibles
- Systems


Technical explanation
=====================

In the SECNode descriptive data, the optional field ``systems`` is introduced.
It is a JSON-Object with the system-names as keys and the URL of the relevant ontology file as the value.
The names of the modules that are part of the system have to be prefixed with the system's name.
For a system to be valid, all modules that are included in the system definition have to be present.

In the description of a module, the field ``definitions`` is a list of URLs that include all relevant
YAML specification files for this module. They do not have to be sorted.

Each URL is of the format: ``https://.../file.yaml#<name>:<version>``.
``name`` and ``version`` must match one of the objects defined in the YAML file.

Example:

.. code:: json

    "cryo1_T": {
        ...
        "definitions": ["url1", "url2"],
        ...
    },
    "systems": {
        "cryo1": "url3",
    }


Disadvantages, Alternatives
===========================

Disadvantages
-------------

Specifying definitions as URLs means that they are not available if there is no
connection to the Internet.

Alternatives
------------

Property format
~~~~~~~~~~~~~~~
An alternative format for the ``systems`` property could look like this:

.. code:: json

    "systems": {
        "cryo1": [
            "url3",
            {
                "T": "cryo1_T",
            },
        ],
    },

This would allow for more flexible names, but introduces another indirection at the same time.

The ontology directly in the description
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
One may include the description of the interfaces directly in the SECNodes description.
This leads to a large overhead, with JSON-formatted descriptions that have to be machine
readable and cover a large number of cases. This also basically leads to a doubling of the
structure, once being described as how the interface would look, and then how it appears.
With a valid system, these would be basically identical, not leading to any advantage.

Therefore, this does not seem sensible.

Open Questions
==============

If there are points that you know have to be discussed/solved, describe them here, maybe with an example.
