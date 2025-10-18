SECoP Issue 13: Access Control (closed)
=======================================

Questions arised whether SECoP should support access restrictions.

Decision
--------

SECoP itself does not provide access control, except the *readonly* parameter property.
Access control should rely on existing network solutions:

- the server might restrict the access to parts of the network
- the server might transport SECoP via an SSL Server
- The server might create multiple *view nodes*, some of them exporting only subsets of
  their modules and/or parameters, and on other restrict access depending on the remote
  address.
