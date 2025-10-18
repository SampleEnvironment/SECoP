SECoP Issue 47: Error classes (closed)
======================================

Motivation
----------

After several (test) implementations it became clear that the currently defined error-classes need to be reworked.
Some classes turned out to not be specific enough, others are completely unused.
Also there are cases where an appropriate error class is completely missing, resulting in abuse of either 'InternalError' or 'Disabled'.


Proposal
--------

redefine the relevant section like:

Error Classes

Error classes are divided into two groups: persisting errors and retryable errors.
Persisting errors will yield the exact same error messge if the exact same request is sent at any later time.
A retryable error may give different results if the exact same message is sent at a later time, i.e.
they depend on state information internal to either the sec-node, the module or the connected hardware.

.. list-table:: persisting errors
    :widths: 20 80

    * - NoSuchModule
      - The action can not be performed as the specified module is non-existent.

    * - NoSuchParameter
      - The action can not be performed as the specified parameter is non-existent.

    * - NoSuchCommand
      - The specified command does not exist.

    * - ReadOnly
      - The requested write can not be performed on a readonly value..

    * - WrongType
      - The requested parameter change or Command can not be performed as the argument has the wrong type.
        (i.e. a string where a number is expected.)
        It may also be used if an incomplete struct is sent, but a complete struct is expected.

    * - RangeError
      - The requested parameter change or Command can not be performed as the argument value is not
        in the allowed range specified by the datatype property.
        This also happens if an unspecified Enum variant is tried to be used, the size of a Blob or String
        does not match the limits given in the descriptive data, or if the number of elements in an array
        does not match the limits given in the descriptive data.

    * - OutOfRange
      - The value read from the hardware is out of sensor or calibration range
      
    * - BadJSON
      - The data part of the message can not be parsed, i.e. the JSON-data is no valid JSON.

    * - NotImplemented
      - A (not yet) implemented action or combination of action and specifer was requested.
        This should not be used in productive setups, but is very helpful during development.

    * - HardwareError
      - The connected hardware operates incorrect or may not operate at all due to errors inside or in connected components.

    * - ProtocolError
      - A malformed Request or on unspecified message was sent.
        This includes non-understood actions and malformed specifiers. Also if the message exceeds an implementation defined maximum size.
        *note: this may be retryable if induced by a noisy connection. Still that should be fixed first!*

.. list-table:: retryable errors
    :widths: 20 80

    * - CommandRunning
      - The command is already executing. request may be retried after the module is no longer BUSY.

    * - CommunicationFailed
      - Some communication (with hardware controlled by this SEC node) failed.

    * - TimeoutError
      - Some initiated action took longer than the maximum allowed time.

    * - IsBusy
      - The requested action can not be performed while the module is Busy or the command still running.

    * - IsError
      - The requested action can not be performed while the module is in error state.

    * - Disabled
      - The requested action can not be performed while the module is disabled.

    * - Impossible
      - The requested action can not be performed at the moment.

    * - ReadFailed
      - The requested parameter can not be read just now.

    * - InternalError
      - Something that should never happen just happened.

:remark: This list may be extended, if needed. clients should treat unknown error classes as generic as possible.


:note: It is to be discussed, if the copy of the origin is really requested to be a full literal copy
    of the origin, a re-serialised version of the de-serialized message or possible even a shortened
    version with just the ``action <space> specifier`` part is sufficient.


Discussion
----------

agreement on its current form on viconf_2019-01-16, needs more examples, close the issue and put into specification