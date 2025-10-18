SECoP Issue 70: message is a JSON object (unspecified)
======================================================

Motivation
----------

encoding, decoding SECoP messages is simplified when
the full SECoP message is a JSON object or JSON array,
as the client anyway needs a JSON library.


Proposal
--------

JSON object
~~~~~~~~~~~

Self speaking, but more verbose.

Examples:

.. code::

  > {"cmd": "describe"}
  < {"cmd": "describing", "data": {...}}
  > {"cmd": "read", "id": ["mod", "param"]}
  < {"cmd": "reply", "id": ["mod", "param"], "data": [295.0, {...}]}
  > {"cmd": "activate"}
  < {"cmd": "activated"}



JSON array
~~~~~~~~~~

This is closer to the current syntax.

Examples:

.. code::

  > ["describe"]
  < ["describing", null, {...}]
  > ["read", "mod:param", {...}]
  < ["reply", "mod:param", [295.0, {...}]]
  > ["activate"]
  < ["activated"]


Remark: technically, it would be possible to support both kinds of syntax.
The client determines which form is to be used.



