.. _error-reply:

``error_*``: Error replies
--------------------------

.. message:: [reply] error_* <specifier> <error-report>

    Contains an error class from the list below as its second item (the
    specifier).  The third item of the message is an :ref:`error-report`,
    containing the request message (minus line endings) as a string in its first
    element, a (short) human readable text as its second element.  The third
    element is a JSON object, containing possibly implementation specific
    information about the error (stack dump etc.).

Examples:

.. code::

    > read tx:target
    < error_read tx:target ["NoSuchModule", "tx is not configured on this SEC node", {}]

    > change ts:target 12
    < error_change ts:target ["NoSuchParameter", "ts has no parameter target", {}]

    > change t:target -9
    < error_change t:target ["RangeError", "requested value (-9) is outside limits (0..300)", {}]

    > meas:volt?
    < error_meas:volt?  ["ProtocolError", "unknown action", {}]


.. _error-classes:

Error classes
-------------

Error classes are divided into two groups: persisting errors and retryable
errors.  Persisting errors will yield the exact same error message if the exact
same request is sent at a later time without other interactions inbetween.

A retryable error may give different results if the exact same message is sent
at a later time, i.e. depends on state information internal to either the SEC
node, the module or the connected hardware.

.. rubric:: Persisting errors

.. errorclass:: ProtocolError

    A malformed Request or on unspecified message was sent.  This includes
    non-understood actions and malformed specifiers.  Also if the message
    exceeds an implementation defined maximum size.  *Note: this may be
    retryable if induced by a noisy connection.*

.. errorclass:: NoSuchModule

    The action can not be performed as the specified module is non-existent.

.. errorclass:: NoSuchParameter

    The action can not be performed as the specified parameter is non-existent.

.. errorclass:: NoSuchCommand

    The specified command does not exist.

.. errorclass:: ReadOnly

    The requested write can not be performed on a readonly value.

.. errorclass:: NotCheckable

    The requested check can not be performed on the specified parameter (i.e. on
    parameters, where no `checkable` property is present, or if it is set to
    false).

.. errorclass:: WrongType

    The requested parameter change or command can not be performed as the
    argument has the wrong type, e.g. a string where a number is expected,
    or a struct doesn't have all required members.

.. errorclass:: RangeError

    The requested parameter change or command can not be performed as the
    argument value is not in the allowed range specified by the `datainfo`
    property.  This also happens if an unspecified enum variant is tried to
    be used, the size of a blob or string does not match the limits given in
    the descriptive data, or if the number of elements in an array does not
    match the limits given in the descriptive data.

.. errorclass:: BadJSON

    The data part of the message can not be parsed, i.e. the JSON data is
    not valid JSON.

.. errorclass:: NotImplemented

    A (not yet) implemented action or combination of action and specifier
    was requested.  This should not be used in productive setups, but is
    very helpful during development.

.. errorclass:: HardwareError

    The connected hardware operates incorrectly or may not operate at all
    due to errors inside or in connected components.

.. rubric:: Retryable errors

.. errorclass:: CommandRunning

    The command is already executing.  The request may be retried after the
    module is no longer BUSY.

.. errorclass:: CommunicationFailed

    Some communication (with hardware controlled by this SEC node) failed.

.. errorclass:: TimeoutError

    Some initiated action took longer than the maximum allowed time.

.. errorclass:: IsBusy

    The requested action can not be performed while the module is BUSY or
    the command still running.

.. errorclass:: IsError

    The requested action can not be performed while the module is in error
    state.

.. errorclass:: Disabled

    The requested action can not be performed while the module is disabled.

.. errorclass:: Impossible

    The requested action can not be performed at the moment.

.. errorclass:: ReadFailed

    The requested parameter can not be read just now.

.. errorclass:: OutOfRange

    The value read from the hardware is out of sensor or calibration range.

.. errorclass:: InternalError

    Something that should never happen just happened.

.. note:: This list may be extended, if needed.  Clients should treat unknown
          error classes as generic errors.
