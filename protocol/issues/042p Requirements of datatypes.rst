SECoP Issue 42: Requirements of datatypes
=========================================

Motivation
----------
Using JSON as part of the transport mechanisms leave a few details about properties of datatypes open.
Especially, the JSON-number format is both used for floating point values and for integers.
Also the JSON-number is agnostic to the datytype used in applications.

So far it is unspecified if there are expectations towards the precision/resolution and range of the
numeric data which is supposed to be transported via SECoP.
This was circumvented by specifiying a C-api datatype. Since there ar emore programming languages around than we can specify a valid datatype for those it makes
sense to move the documentation of a specific C-api elsewhere.

That would leave the problem of having undefined value ranges/precisions in SECoP.

It is upto now unclear if SECoP should even define something or leve this to the application.
Also, one of the design desicions was to optimize SECoP to allow low-end hardware act as SEC-node.
Insofar it is contradictory for a low-end microcontroller or PLC to have to use 64bit double floating formats,
especially, if a 24 bit fixed point format would have sufficed!

On the ECS-side there seem to be no obvious problem with specifying to use 64 bit representations, as these machines are always assumed to be powerful enough.
Since the SEC-node can present limits on the used doulbe and int values, the required range to be supported can be deduced by an ECS.
It then MUST NOT send values outside that range wich is exactly what the current specification of double ind int datatypes requests.
So the missing information of the ECS is the resolution, the SEC-node can handle.
As successful requests to the SEC-node return a reply containing the actually used value,
the ECS can deduce the useable resolution of the SEC-node.

Proposal
--------
The specification of the C-api of the HZB-dll should be moved to a separate document.

It should be discussed what value range/resolution is really expected in SECoP.
So far, no modules were found which require more than 3 subdigits or a value range bigger than 0...2000.

A text similiar to the following should be included in the specification:

  The ECS SHOULD send floating point values with the full precision its implementation allows,
  the SEC-node uses as many digits as it can handle and ignores the rest.
  In the reply, the SEC-node uses as many digits, as its implementation allow.
  Also, if an SEC-node uses representations smaller than a predefined (say 64 bit double) one, it MUST
  specify all double or int datatypes with the acceptable limits in the descriptive data.
  (Otherwise, it SHOULD fully specify number ranges.)

  An ECS MUST support AT LEAST the full 32bit float value range and precision,
  but SHOULD use 64bit double values internally.


Discussion
----------
Discussion is clearly needed.
No detailed discussion of the proposal so far.

It may also be necessary to adapt the definition of the datatypes by removing the unlimited option,i.e.
the numeric datatytpes could be ALWAYS restricted.
