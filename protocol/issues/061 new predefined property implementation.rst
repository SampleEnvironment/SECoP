SECoP Issue 61: new predefined property implementation (proposed)
=================================================================

Proposal
--------

A new module property 'implementation' describing details of the implementation.
For example the implementing class in a python framework.

we add the following to the optional module properties:

``"implementation"``:
     a string indicating information about the implementation of the module, like a python class.

we add the following to the optional SEC node properties:

``"implementation"``:
     a string indicating information about the implementation of the SEC node, like the used library or framework, including its version.

Discussion
----------

We should skip the The SEC node property "implementation" as it is already covered by "firmware".

Decision
--------

At vidconf from 2021-11-30:

We add the following to the optional module properties:

``"implementation"``:
     A string indicating information about the implementation of the module, like a python class.

     example: ``"secop_psi.ppms.Field"``
