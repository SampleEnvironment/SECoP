SECoP Issue 31: Clarify timing requirements of status updates
=============================================================

The problem
-----------
An often task in an ECS is to 'move' a device or module to a new value and wait
until this is done.
For this purpose SECoP defines the status parameter tuple with an easy to
interpret status-code.
And while it is easy for an ECS to find the end of an initiated action when the
status-code changes from BUSY to something else,
it may miss the start of the BUSY phase (i.e. when staus-code goes to BUSY) if
the initiated action is short.
As SECoP only specifies that commands initating an action should 'be short' and
the BUSY state be update via updates on the status parameter,
the correct sequence keeps unspecified.

Actually we can have two problems here:
  1) an polling ECS may miss the BUSY state if it's polling period is longer
     than the duration of the initiated action.
     This may lead to an endless wait, if the ECS expected a BUSY state.

  2) a SEC-Node taking a while to report a BUSY status-code may fool an ECS
     into thinking the initiated action is already over.
     This may lead to the ECS continuing too early, leading to measurements
     with the wrong conditions.

To maximise the interoperability between different implementations, both
problems MUST be avoided. Thankfully there is an easy solution.

The SEC-Node needs to set the status-code to BUSY **before** the reply of the
initiating action (``do`` or ``change``) is sent by the SEC-Node.
As always, update messages for all subscribed connections should be sent as
soon as the status parameter is changed (i.e. **before** the reply is sent).


Proposal
--------
Enrico proposes to specify the following sequence to be followed for initating
an action in a module:

sequence:
  1) ECS checks that the action can be carried out (i.e. status-code is neither BUSY nor ERROR)

  2) ECS sends the initiating message request (either ``change`` target or ``do`` go).

  3) SEC-Node checks if anything is actually to be done. if not, continue to point 5)

  4) SEC-Node 'sets' the status-code to BUSY and instructs the hardware to execute
     the requested action.
     Also an ``update`` status event (with the new BUSY status-code) MUST be sent
     to ALL subscribed clients (if any).
     From now on all read requests will also reveal a BUSY status-code.

  5) SEC-Node sends the reply to the request of point 2) indicating the success of the request.

     note: This may also be an error. In that case point 4) was probably not taken.

     note: An error may be replied after the status was sent to BUSY:
     if triggering the intended action failed (Communication problems?).

  6) An event based ECS which may process the ``update`` message from point 4)
     after the reply of point 5) MUST query the status parameter synchronously
     to avoid the race-condition of missing the (possible) BUSY status-code.

  7) when the action is finally finshed and the module no longer to be considered BUSY,
     an ``update`` status event MUST be sent, also subsequent status queries
     should reflect the now no longer BUSY state.

Following this sequence of events, both above reported problems can not appear,
maximising interoperability.


After thoughts
--------------
This problem is an essential problem for all actions influencing more than one parameter.
As such it should be applied to a broader scope: whenever an request would
influence another parameter/command than the addressed one, those 'side-effects'
MUST to be broadcast (+ internally reflected for polling clients)
**before** the reply is sent by the SEC-node.


Discussion
----------

video conference at 2018-10-04
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

General agreement, but not (yet) finally decided as accepted.

Comment by Markus:
  I agree fully on the subject, I just wonder if we can not describe the essential thing
  a little shorter:

    The SEC-Node MUST NOT send an update message indicating a <module>:status not BUSY
    after the reply to an action (change <module>:target or do <module>:go), but before
    the initated action is finished.

  After that we might still add the sequence proposed by Enno. Point (1) of the sequence might make sense,
  but is not really required. In case the status is BUSY or ERROR, the reply will be an error message.
  By the way - did we specifiy that the target can not be changed while BUSY? Unless there are good
  reason aginst I would allow it. The implementation might report an error, if it does not support it.

  There is an other possible problem in synchronous mode: when polling is slow, an IDLE status
  may be missed if the next action is issued by an other client. In order to avoid this, the
  first client might want to poll the target value too and check if it was changed.


video conference at 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Comment by Markus:
    sequence: point (1) should be skipped.

    There might be SEC-Nodes allowing to change the target while running.

    If the ECS wants to change any parameter (including target) it should just try - and react on the IsBusy error message accordingly, either by waiting for idle, by sending a stop command first, or by raising an error condition, depending on the context.

    I do not think that it is a good idea to let the ECS remember a parameter change to be done later, when the module is no longer busy.

Comment by Klaus:
    To point 6 in the sequence: shouldn't we demand that the ECS must process all messages from one SEC-node in the right temporal order?

Comment by Enrico:
    point 1 is not strictly needed, point 6 should be formulated more clear. point 6 avoids a rare problem if an ECS uses two connections, one with and one without activation, and the message handling code is written in a simple-minded way.

Agreement on the above proposal, closing this issue.
A shortened version of the above is to be included in the specification. It should also contain some message examples (to show the expected order of events).


