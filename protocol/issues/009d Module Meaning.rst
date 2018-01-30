SECoP Issue 9: Module Meaning (under discussion)
================================================

meaning
-------

For the ECS an automatic detection of the main modules would be desirable.

For example a SEC Node could tell which sensor would be the closest one to
the sample, which should be registered in the ECS as the sample temperature.

For this, a module property "meaning" is proposed. This can get one of the
following values:

* sample_temperature
* magnetic_field

importance
----------

But what to do, if several modules claim to be the sample temperature?
There might be a SEC node controlling cryostat, which has a sample temperature sensor,
but another SEC node controlling an insert with a sample sensor. As the insert
is put into the cryostat, we declare the cryostat to have importance=1 and
the insert an importance=2. To resolve the ambiguity, the ECS chooses the
module with the highest importance to be labelled as the read sample temperature.

Proposal:

predefined importance:

 * importance=1 for a device which can not be inserted or added to another one
 * importance=2 for a device which can be inserted into an other one

Higher values are to be used when an additional device may be inserted into an insert
and the like.

We should allow importance to be a floating point number, in case later a value
between 1 and 2 has to be used.
