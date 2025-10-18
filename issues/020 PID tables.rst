SECoP Issue 20: PID tables (unspecified)
========================================

Some devices may not just 'go' to a target, instead they represent a measured value which
can be influence by some 'activity'. Very often the relationship of activity vs. measured value
or even change of measured value is (strongly) nonlinear, making it impossible to derive the needed
amount of 'activity' to reach a certain desired value. In those cases a regulation algorithm is often used.
Most common exampoles are PID-control, but also two-point regulations or other algorithms may be implemented.
These algorithms typicall have some parameters, influencing their working, which may
depend on the desired value (or others).
In thoses cases, these values are often put into tables, where the actual row to be used is derived in some way from the measured values.
There are so called 'zone' tables, listing the limits for the measured value in which a certain row is to be applied,
and interpolation tables, which essentially contain the poptimized regulation parameters AT some
measured values, interpolating the parameters for values inbetween.
Of course, mixed forms are possible.

We need to:

- define such a table (idea: an array of a struct, containing the 'selector' value as well
  as the regulation parameters (see `Issue 067`_).
- ideally be able to access it not just as a whole, but also just a single row, or even just a single element.
- consider that there may be more than a single selector, or even multiple tables
- respect that implementors may want to be able to 'enable/disable' a certain table or row
- be aware that also tables for other purposes may be present.


Proposal
--------
There a different ideas:

new datainfo type 'table'
+++++++++++++++++++++++++

We may add a new table datainfo type, containing:

- the (maximum/fixed) number of rows
- the columns of that table (may be a struct)
- information about which elements are the selectors and how they work (zone/interpolate/other?)

using an array
++++++++++++++

A table may also be defined as an array of structs.
This has the following problems:
- no way to specify the selectors or how they work
- currently, struct-items have neither description, nor properties like readonly

accessing the elements of a table
+++++++++++++++++++++++++++++++++

Since reading/changing the whole table at once is already supported, we to define methods for accessing rows and elements:

- read_row(tablename, rownumber) -> struct(...) # the row
- change_row(tablename, rownumber, struct(...))
- read_cell(tablename, rownumber, columname) -> cell_content
- change_cell(tablename, rownumber, columname, new_cell_content)

The biggest problem being the cell accessing methods due to the fact, that the datatype of the cells may vary
more than the type system of SECoP allows (i.e. mixing bool values for enable/disable with float values for the actual parameter)

Or we extend the protocol in a way which allows accessing sub-items of parameters (another issue).

Discussion
----------
to be done


.. _`Issue 067`: 067%20pid%20control%20parameters.rst