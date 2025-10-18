SECoP Issue 54: Splitting _update_ into two Message Types (closed)
==================================================================

Motivation
----------

In the discussion about error *update* replies, we realized that it might not
be a good idea, that the *update* message is used both as a reply to a *read*
message and as an asynchronous value update.

Proposals for new read request / reply message names
----------------------------------------------------

    * read, reply
    * read, value
    * read, readreply
    * read, read (a request can be distinguished from a reply, by the missing 3rd element)
    * query, reply
    * query, read
    * query, queried
    * pick, picked

Discussion
----------

We should define more precisely the meaning of a *read* request. Is mandatory, that
parameters stored in hardware, are always read from hardware for the read reply?

If yes, we need two different messages as proposed above, because the client can not know,
if an update message was sent in parallel with the read request, and the client interpretes
this as the answer.

If no (preferred by Markus), the only thing which has to be guaranteed, is what Enno
proposed: all side effects on a *change* command have to be realized (i.e. influenced
parameters updated) before the corresponding *changed* message. For clarity reasons
we still might choose to have different names for read reply and async update.

Decision on vidconf_2019-03-13
------------------------------
   
The reply to `read` is to be renamed to `reply` instead of `update`.
Agreement on this.
The discussion related to Issue 45 (Async Error Updates) about the format
of the 'error' message will continue.

