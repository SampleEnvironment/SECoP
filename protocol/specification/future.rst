.. _future-compatibility:

Future compatibility
====================

Notes for implementors of the current specification
---------------------------------------------------

* As JSON can not handle non-numerical quantities like 'Inf' or 'NaN', either an
  appropriate error message should be generated, or the closest representable
  numerical value (+/- double_max?) should be used.

* All values transferred between ECS and SEC node should be validated on both
  sides.  This may be relaxed in future specifications.  Errors, which arise
  from the validation of SEC node values on the ECS-side should not crash the
  ECS and should inform the user about this violation of specification along
  with the data, the validator and why validation failed.


Foreseen extension mechanisms
-----------------------------

The herein specified protocol has foreseen some extension mechanisms in its
design:

* Add messages, keeping the 'triple' structure of message/specifier/data.

  .. note:: That is why custom messages MUST be prefixed with an underscore.

* Extend specifiers with ':' separated identifiers, getting more and more
  specific.

  An empty string as specifier addresses the SEC node, ``<module>`` addresses a
  module, and ``<module>:<accessible>`` addresses an accessible of a module.

  If there will ever be such things as node-accessibles, they will be addressed
  as ``:<accessible>``.  Also properties may be addressed like
  ``<module>:<accessible>:<property>``.

  In the same sense as an empty string selects the whole SEC node, ``<module>:``
  may at some point select ALL parameters of a module.

* Define additional parameter/command/property names.

  .. note:: That is why custom parameters, commands and properties MUST be
            prefixed with an underscore.

* Extend reports (only append to them, never changing the already defined
  fields).

  The structure report may need to be nested inside a JSON array in the future,
  should we need to extend that.

* Use so far unused data fields (there are not so many).

* Define additional status groups or statuscodes.

* Define additional interface classes.

* Define additional features, listed in an additional property.


Message handling
----------------

For this, see :ref:`this section <message-compat>`.


Binary representations of the protocol
--------------------------------------

So far only the above described, textual protocol is defined.  Since this is not
optimal for bandwidth limited connections (e.g. RS232), a shorter, binary
representation may be developed.  This will essentially keep the structure of
the messages, but replace the components of a message with shorter, binary
representations.

Good candidates for this are CBOR (see :rfc:`7049`) and MessagePack (see
https://msgpack.org/).
