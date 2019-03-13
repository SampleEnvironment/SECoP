SECoP Issue 53: Custom Status Values (closed)
=============================================

Motivation
----------

Status values of the form x0z have either predefined meanings or are reserved,
status values xyz with 1<=y<=9 are forseen for custom use. If both the ECS and
the SEC-Node are from the same implementor, this is no problem.
But an ECS may not know per se the meaning of custom status values, if it does
no know from which implementor the SEC Node is.

Discussion
----------

At the vidconf_2019-02_20 this issue was discussed, with several possible solutions:

Rejected Proposals:

1) An ECS must only interprete a custom status value xyz, if the name of the status
   matches. Otherwise it must treat it the same as status x00.
   Collision risk is much lower with names than with numbers.

2) Central registry for custom status numbers

Proposal proposed at above vidconf:

3) A SEC-Node or module property 'implementor', defining the implementor facility
   or manufacturer. The ECS must interprete custom status values xyz as general x00,
   if does not know, what the indicated implementor specified as their meaning.
   
   As a module might be a proxy to a module on another SEC-node, this information
   should be on the module instead of the SEC-node.

Proposal by Markus:

4) Instead of a separate property, add an item to the features property. The item
   must start with a facility name (for example "psi.ch.sinqfeatures").
   You might say, that this is not what features were made for, but if we think
   general, it is not really different. We always said, that features are not
   just a list of mandatory parameters, but also a specification of some behaviour,
   which might not be known otherwise just from the description.
   Such a feature might not only include the meaning of status values, but also
   custom properties.
   

Decision on vidconf_2019-03-13
------------------------------
   
There should only be a single implementor per *module*.
Discussion about the format of it and how to format an implementor.
The implementor should be globally unique.

The specification may need a (short) section about how to generate the required uniqueness of the names.




