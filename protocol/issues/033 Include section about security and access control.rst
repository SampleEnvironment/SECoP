SECoP Issue 33: Include section about security and access control (closed)
==========================================================================

The problem
-----------
SECoP handles intentionlly no security or access control concerns.

While this is no problem per se, it is advisable to have at least a section explaining it and
to list a few examples on how this can be achieved outside SECoP.

.. note:: completeness is not needed here.


Proposal
--------
Enrico proposes to include a section called "security and access control"
stating the committees decision and list a few examples how both can be achieved
by other means.

One example could be to use SECoP over openTLS (see :RFC:`8446`) to encrypt
communication and restrict access to those clients having the right certificates
(depending on the TLS configuration).

Another example could a filter, which disallow certain accesses.
Note that in this case the descriptive data MUST reflect the allowed actions,
i.e. will differ from that of the unfiltered access.

Other solutions exist, but can not exhaustively listed here.

While these solutions normally only work on the whole node
(by granting or denying possibly encrypted access),
a SEC-Node may also provide multiple contact points, allowing different access levels.
i.e. there could be 'master' view with unrestricted access and a limited view
with access restrictions.
As these are largely implementation specific and not different
on the protocol level, these cases are not covered in the SECoP protocol.


Discussion
----------
Discussion usually settled quickly on 'It's not SECoP's task to provide
encryption or access control'.

video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Decisions:
 - proposal accepted, a section should be included in the specification.
 - Another Section about datatransfer should also be included.
 - Issue is to be closed.


