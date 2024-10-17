.. _future-compatibility:

Future Compatibility - notes for implementors
=============================================

notes for implementors of current specification
-----------------------------------------------

# As JSON can not handle non-numerical quantities like 'Inf' or 'NaN',
  either an appropriate error message should be generated, or the closest representable
  numerical value (+/- double_max?) should be used.

# all values transferred between ECS and SEC node should be validated on both sides.
  This may be relaxed in future specifications.
  Errors, which arise from the validation of SEC node values on the ECS-side should not crash the ECS
  and should inform the user about this violation of specification along with the data, the validator and why validation failed.

Foreseen extension mechanisms
-----------------------------

The herein specified protocol has foreseen some extension mechanisms in its design:

* add actions, keeping the 'triple' structure of action/specifier/data

  :Note:
      That is why custom actions MUST be prefixed with an underscore.

* extent specifier with ':' separated identifiers, getting more and more specific

  An empty string as specifier addresses the SEC node, ``<module>`` addresses a module,
  and ``<module>:<accessible>`` addresses an accessible of a module.

  If there will ever be such things as node-accessibles, they will be addressed as ``:<accessible>``.
  Also properties may be addressed like ``<module>:<accessible>:<property>``.

  In the same sense as an empty string selects the whole SEC node, ``<module>:`` may select ALL parameters of a module.

* define additional parameter/command/property names

* extend reports (only append to them, never changing the already defined fields)

  :Note:
      The structure report may need to be nested inside a JSON-array in the future, should we need to extend that.

* use so far unused datafields (there are not so many).

* define additional status groups or statuscodes

* define additional interface classes

* define additional features, being listed in an additional property

Message Handling
----------------

For this, see :ref:`message-overview`.

Binary representations of the protocol
--------------------------------------

so far only the above described, textual protocol is defined.
Since this is not optimal for bandwidth limited connections (e.g. RS232), a shorter, binary representation
may be developed. This will essentially keep the structure of the messages, but replace the components
of a message with shorter, binary representations.

Good candidates for this are CBOR (see :RFC:`7049`) and MessagePack (see https://msgpack.org/).
