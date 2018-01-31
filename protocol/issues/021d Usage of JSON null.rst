SECoP Issue 21: Usage of JSON null
==================================

Proposal
--------

Usage on null values in *update* messages
#########################################

On an *activate* request, the SEC node has to send *update* messages for the values of
all parameters, before sending the *active* message. The *update* replies might
contain null values for the following cases:

* the implementation is designed not to trigger readings on an *activate* request, and
  the value is not polled yet
* the hardware is not initialized yet
* there was an error on an earlier reading.

However, on a *read* request, the reply should be either a valid (non null) value,
or an *error* reply.


Usage on null values in *change* messages
#########################################

If some elements of a complex datatype are to be changed, but other elements are not
to be changed, a *change* message might be allowed to contain *null* for all
unchanged elements.

Discussion
----------

Usage on null values in *update* messages
#########################################

Is it allowed that a complex datatype contains *null* elements in an update message,
in case the data is only partly invalid? What about replies to *read* messages with
a complex datatype? In this case, an error message would suppress the information
about the valid elements ...
  

Usage on null values in *change* messages
#########################################

Why may we need this feature? 2 possible reasons:

1) efficiency: not having to send commands to the hardware for unchanged values
2) avoiding race conditions: two clients, changing each a different element in the
   same complex structure - one change may be overridden

If we allow this, the implementation on *all* SEC Nodes with complex datatypes
gets more complicated. Alternatives:

1) a sophisticated SEC Node may compare the new values with the old ones, and send only the changed elements.
2) split a complex parameter into different parameters.

Opinions:
---------

Markus: do not allow *null* in *change* messages, or introduce a property *allow_null_elements*=true on the parameters which accept this behaviour.

