SECoP Issue 46: remote logging
==============================

Motivation
----------

At least for debugging it is fortunate to transfer log messages from the SEC-node to the ECS.
This may already be implemented with custom messages.
This proposal shall collect ideas and implementations and discuss if and which pieces may or will go to the standard.


Proposal
--------

Based on what seems to be the most generic assumption of having three actual logging levels ``debug``, ``info`` and ``error``,
and wanting to switch the logging off completely, the following ideas arise:

Idea a)
+++++++

Introduce two new actions: ``logging`` and  ``log``.

``logging``:
  followed by a specifier of <modulename> and a string in the JSON-part which is either "debug", "info", "error" or is the JSON-value false.
  This is supposed to set the 'logging level' of the given module (or the whole SEC-node if the specifier is empty) to the given level:

  This scheme may also be extended to configure logging only for selected paramters of selected modules.

  :false:
    Remote logging is completely turned off.
  :"error":
    Only errors are logged remotely.
  :"info":
    Only 'info' and 'error' messages are logged remotely.
  :"debug":
    All log messages are logged remotely.

  A SEC-node should reply with an error-report (``protocolerror``) if it doesn't implement this message.
  Otherwise it should mirror the request, which may be updated with the logging-level actually in use.
  i.e. if an SEC-node does not implement the "debug" level, but "error" and "info" and an ECS request "debug" logging, the
  reply should contain "info" (as this is 'closer' to the original request which than "error" or ``false``).
  Similiarly, if logging of a too specific item is requested, the SEC-node should activate the logging on the
  least specific item where logging is supported. e.g. if logging for <module>:<param> is requested, but the SEC-node
  only support logging of the module, this should be reflected in the reply and the logging of the module is to be influenced.

  Note: it is not foreseen to query the currently active logging level. It is supposed to default to ``false``.

``log``:
  followed by a specifier of <modulename>:<loglevel> and the message to be logged as JSON-string in the datapart.
  This is an asynchronous event only to be sent by the SEC-node to the ECS of it activated logging.


example::

  > logging  "error"           ; note: empty specifier -> select all modules
  < logging  "error"           ; SEC-node confirms
  < log mod1:debug "polling value"
  < log mod1:debug "sending request..."
  ...

another example::

  > logging mod1 "debug"       ; enable full logging of mod1
  < logging mod1 "error"       ; SEC-node can only log errors, logging of errors of mod1 is now active
  < log mod1:error "value par1 can not be determined, please refill read-out liquid"
  ...
  > logging mod1 false
  < logging mod1 false



Discussion
----------

Note:
  this is remotely connected to Issue 45: Out of band signalling

not discussed in its present form.

to be implemented in FRAPPY as custom extension (actions prefixed with ``_``).

Decision from vidconf 2018-12-03
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

accepted in it precision form

ready to be closed?
