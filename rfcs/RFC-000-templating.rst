- Feature: Linking to Ontologies
- Status: Open
- Submit Date: < your date >
- Type: Protocol
- PR: <link to the pull-request>
- Implementation Tracking: -

Summary
=======

This Feature introduces a way to include information about Module !INTERFACES! and systems of multiple modules in the description of a SECNodes module.
This is achieved by linking to the append-only definitions introduced in RFC-XXX.

Goal
====

Interface classes and !system-interfaces! should be accessible in a central repository.
SECNodes should link to their respective specifications in order to enable three things:

- the implementors of clients can get a description of the functionality of these interface classes.
- a client can use the extended description to provide further functionality
- the structure of the SECNode can be verified to follow the interface


Technical explanation
=====================


In the SECNodes description, the optional field ``systems`` is introduced.
It is a JSON-Object with the system-names as keys and the url of the relevant ontology file as the value.
The names of the modules that are part of the system have to be prefixed with the systems name.
For a system to be valid, all modules that are included in the !spec! have to be present.

In the description of the Module, the field ``xxx`` is a list of urls that include all relevant !spec! files for this module.
They do not have to be sorted.

Each url is of the format: ``<blub>/folder/file#<!interface!>:<version>``.
The first query parameter specifies which of the interfaces found in the file is relevant to this module.
The second parameter specifies the ``version`` of the !spec!.

Example:

.. code:: json

    "cryo1_T": {
        ...
        "xxx": ["url1", "url2"],
        ...
    },
    "systems": {
        "cryo1": "url3",
    }

Disadvantages, Alternatives
===========================

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
This leads to a large overhead, with JSON-formatted descriptions that have to be machine readable and cover a large number of cases.
This also basically leads to a doubling of the structure, once being described as how the interface would look, and then how it appears.
With a valid system, these would be basically identical, not leading to any advantage.

Therefore, this does not seem sensible.

Open Questions
==============

If there are points that you know have to be discussed/solved, describe them here, maybe with an example.
