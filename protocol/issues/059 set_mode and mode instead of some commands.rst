SECoP Issue 59: set_mode and mode instead of some commands (closed)
===================================================================

.. contents::
    :depth: 3
    :local:
    :backlinks: entry

Motivation
----------

Combining the information about flow control with other status values might not be a good idea, as:

* it is not easily extensible, if in future we need more states
* in the current model, for each target state, we need a command, where for the proposed model,
  we have a parameter op_mode instead, which takes whe same number of values
* in the current model, we do not have the information of the wanted final state. As long
  as there is only one client, this seems not important. But with multiple clients it is
  a benefit.

Proposal
--------

Introduce a mode (readonly) / op_mode (writable) parameter instead of only a mode (writable)
parameter, and skip all of prepare, finalize, shutdown.

* predefine at least the following codes for op_mode: DISABLED, STANDBY, PREPARED
* predefine at least the following codes for mode: DISABLED, STANDBY, PREPARED, INITIALIZING, PREPARING, MOVING, STABILIZE, FINALIZE
* reduce the predefined status codes to: DISABLED, IDLE, WARN, UNSTABLE, BUSY, PREPARING, MOVING, FINALIZE, ERROR

FINALIZE is still needed, because the ECS will have know when to start the measurement in
classical neutron scattering mode.

Discussion
----------

Let us take the example of a cryomagnet and compare tow models A and B.

Cryomagnet Example
..................


model A
~~~~~~~

Model decided in May 2019 in Munich.
The status code contains all information of "flow control". There information
about the final state is lacking.

* parameters: value, status, target, mode
* commands: stop, prepare, finalize, shutdown
* possible status codes: DISABLED, IDLE, PREPARED, WARN, WARN_UNSTABLE, BUSY, PREPARING, PREPARING_UNSTABLE, MOVING, STABLIZE, FINALIZE, ERROR
* possible mode values: AUTO, MANUAL

model B
~~~~~~~

Proposed model. The status code contains only the information needed for the classical experiment.
After a change target the ECS wait before starting the measurement until the status code is either
FINALIZE, IDLE or WARN (or any substates of them).
For the special case, where one wants to measure while MOVING the op_mode/mode parameter
has to be used/examined.

* parameters: value, status, target, mode, op_mode
* commands: stop
* possible status codes: DISABLED, IDLE, BUSY, FINALIZE, WARN, ERROR
* possible op_mode values: DISABLED, STANDBY, PREPARED
* possible mode values: DISABLED, STANDBY, PREPARED, INITIALIZING, PREPARING, MOVING, STABILIZE, FINALIZE

explanations for the following tables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The status text is choosen to use terms related to a cryomagnet, whereas the other
codes are using a more general wording.

In a row with an action, all other columns take their value before the reply message to the action is communicated.
In between the rows, always some time pass.

No `go` command is assumed. In case themodule is implemented with a `go` command, just send
a `go` after any `change target` in model A, and after any `change target` and `change op_mode`
in model B. In model B, when doing a `change target` and a `change mode` togther, only send
one `go` command.

initialize the module
~~~~~~~~~~~~~~~~~~~~~

+---------------------------+--------------------------------------------------+------------+
|model A                    |model B                                           |both        |
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|action    |mode|status code|action          |op_mode |op_state    |status code|status text |
+==========+====+===========+================+========+============+===========+============+
|          |AUTO|DISABLED   |                |DISABLED|DISABLED    |DISABLED   |shut down   |
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|initialize|AUTO|BUSY       |op_mode:=STANDBY|STANDBY |INITIALIZING|BUSY       |initializing|
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|          |AUTO|IDLE       |                |STANDBY |STANDBY     |IDLE       |zero field  |
+----------+----+-----------+----------------+--------+------------+-----------+------------+

change target (normal case)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* initial state: persistent, value=1
* wanted state: persistent, value=2

+--------------------------+----------------------------------------+-----------+
|model A                   |model B                                 |both       |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|action   |mode|status code|action   |op_mode |op_state |status code|status text|
+=========+====+===========+=========+========+=========+===========+===========+
|         |AUTO|IDLE       |         |STANDBY |STANDBY  |IDLE       |persistent |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|target:=2|AUTO|PREPARING  |target:=2|STANDBY |PREPARING|PREPARING  |leads up   |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|PREPARING  |         |STANDBY |PREPARING|PREPARING  |heat sw    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|MOVING     |         |STANDBY |MOVING   |MOVING     |ramping    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|STABILIZE  |         |STANDBY |STABILIZE|BUSY       |stabilize  |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|FINALIZE   |         |STANDBY |FINALIZE |FINALIZE   |cool sw    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|FINALIZE   |         |STANDBY |FINALIZE |FINALIZE   |leads down |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|IDLE       |         |STANDBY |STANDBY  |IDLE       |persistent |
+---------+----+-----------+---------+--------+---------+-----------+-----------+

prepare (go to driven state)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+------------------------+-------------------------------------------------+-------------+
|model A                 |model B                                          |both         |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|action |mode|status code|action            |op_mode |op_state |status code|status text  |
+=======+====+===========+==================+========+=========+===========+=============+
|       |AUTO|IDLE       |                  |STANDBY |STANDBY  |IDLE       |idle         |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|prepare|AUTO|PREPARING  |op_mode :=PREPARED|PREPARED|PREPARING|FINALIZE   |leads up     |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|       |AUTO|PREPARING  |                  |PREPARED|PREPARING|FINALIZE   |heat sw      |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|       |AUTO|PREPARED   |                  |PREPARED|PREPARED |IDLE       |driven stable|
+-------+----+-----------+------------------+--------+---------+-----------+-------------+

change target (time saving case)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* initial state (as final state above): prepared (driven), value=2
* wanted state: prepared (driven), value=3

+-------------------------------+----------------------------------------+--------------+
|model A                        |model B                                 |both          |
+------------+------+-----------+---------+--------+---------+-----------+--------------+
|action      |mode  |status code|action   |op_mode |op_state |status code|status text   |
+============+======+===========+=========+========+=========+===========+==============+
|            |AUTO  |PREPARED   |         |PREPARED|PREPARED |IDLE       |driven stable |
+------------+------+-----------+---------+--------+---------+-----------+--------------+
|mode:=MANUAL|MANUAL|PREPARED   |         |        |         |           |driven stable |
+------------+------+-----------+---------+--------+---------+-----------+--------------+
|target:=3   |MANUAL|MOVING     |target:=3|PREPARED|MOVING   |BUSY       |ramping       |
+------------+------+-----------+---------+--------+---------+-----------+--------------+
|            |MANUAL|STABILIZE  |         |PREPARED|STABILIZE|BUSY       |stabilize     |
+------------+------+-----------+---------+--------+---------+-----------+--------------+
|            |MANUAL|PREPARED   |         |PREPARED|PREPARED |IDLE       |driven        |
+------------+------+-----------+---------+--------+---------+-----------+--------------+

go to STANDBY (persistent) state
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------+----------------------------------------------+-------------+
|model A                    |model B                                       |both         |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|action  |mode  |status code|action          |op_mode |op_state|status code|status text  |
+========+======+===========+================+========+========+===========+=============+
|finalize|MANUAL|FINALIZE   |op_mode:=STANDBY|STANDBY |FINALIZE|FINALIZE   |cool sw      |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|        |MANUAL|FINALIZE   |                |STANDBY |FINALIZE|FINALIZE   |leads down   |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|        |MANUAL|IDLE       |                |STANDBY |STANDBY |IDLE       |driven stable|
+--------+------+-----------+----------------+--------+--------+-----------+-------------+

change target with predefined final state STANDBY (persistent)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* initial state: prepared (driven), value=1
* wanted state: persistent (STANDBY), value=2
* Remark for model B only: if the SEC Node does not accept op_mode while BUSY, wait until IDLE before changing op_mode
* if the module has a `go` command, change mode/op_mode and target before sending `go`

+-----------------------------+-----------------------------------------------+-------------+
|model A                      |model B                                        |both         |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|action    |mode  |status code|action          |op_mode |op_state |status code|status text  |
+==========+======+===========+================+========+=========+===========+=============+
|          |MANUAL|PREPARED   |                |PREPARED|PREPARED |IDLE       |driven stable|
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|mode:=AUTO|AUTO  |PREPARED   |                |        |         |           |driven stable|
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|target:=2 |AUTO  |MOVING     |target:=2       |PREPARED|MOVING   |MOVING     |ramping      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |           |op_mode:=STANDBY|STANDBY |MOVING   |MOVING     |ramping      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |STABILIZE  |                |STANDBY |STABILIZE|BUSY       |stabilize    |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |FINALIZE   |                |STANDBY |FINALIZE |FINALIZE   |cool sw      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |FINALIZE   |                |STANDBY |FINALIZE |FINALIZE   |leads down   |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |IDLE       |                |STANDBY |STANDBY  |IDLE       |persistent   |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+

change target with predefined final state prepared
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* inital state (as final state above): STANDBY(persistent), value=2
* wanted state: prepared (driven), value=3
* Remark for model B only: if the SEC Node does not accept change target while BUSY, wait until IDLE before changing target
* if the module has a `go` command, change mode/op_mode and target before sending `go`

+-------------------------------+-------------------------------------------------+-------------+
|model A                        |model B                                          |both         |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|action      |mode  |status code|action            |op_mode |op_state |status code|status text  |
+============+======+===========+==================+========+=========+===========+=============+
|            |AUTO  |PREPARED   |                  |STANDBY |STANDBY  |IDLE       |persistent   |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|mode:=MANUAL|MANUAL|PREPARED   |                  |        |         |           |persistent   |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|target:=1   |MANUAL|PREPARING  |op_mode :=PREPARED|PREPARED|PREPARING|BUSY       |leads up     |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |      |           |target:=3         |PREPARED|PREPARING|BUSY       |leads up     |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|PREPARING  |                  |PREPARED|PREPARING|BUSY       |heat sw      |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|MOVING     |                  |PREPARED|MOVING   |MOVING     |ramping      |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|STABILIZE  |                  |PREPARED|STABILIZE|BUSY       |stabilize    |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|PREPARED   |                  |PREPARED|PREPARED |IDLE       |driven stable|
+------------+------+-----------+------------------+--------+---------+-----------+-------------+

shut down
~~~~~~~~~

* inital state: persistent (STANDBY), value=2

+-------------------------+-------------------------------------------------+-----------+
|model A                  |model B                                          |both       |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|action  |mode|status code|action            |op_mode |op_state |status code|status text|
+========+====+===========+==================+========+=========+===========+===========+
|        |AUTO|IDLE       |                  |STANDBY |STANDBY  |IDLE       |persistent |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|shutdown|AUTO|PREPARING  |op_mode :=DISABLED|DISABLED|PREPARING|BUSY       |leads up   |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|        |AUTO|PREPARING  |                  |DISABLED|PREPARING|BUSY       |heat sw    |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|        |AUTO|MOVING     |                  |DISABLED|MOVING   |BUSY       |ramping    |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|        |AUTO|STABILIZE  |                  |DISABLED|STABILIZE|BUSY       |stabilize  |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|        |AUTO|FINALIZE   |                  |DISABLED|FINALIZE |BUSY       |cool sw    |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|        |AUTO|DISABLED   |                  |DISABLED|DISABLED |DISABLED   |shut down  |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+


Options
.......

a) Commands: shutdown, prepare, finalize, 13 Status codes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following status values:

+-----------------------+--------------+
|status                 |use cases     |
+----+------------------+----+----+----+
|code|name              |wait|meas|ramp|
+====+==================+====+====+====+
|0   |DISABLED          |    |    |    |
+----+------------------+----+----+----+
|100 |IDLE              |    |meas|    |
+----+------------------+----+----+----+
|110 |PREPARED          |    |meas|    |
+----+------------------+----+----+----+
|200 |WARN              |    |meas|    |
+----+------------------+----+----+----+
|250 |WARN_UNSTABLE     |    |    |    |
+----+------------------+----+----+----+
|300 |BUSY              |wait|    |    |
+----+------------------+----+----+----+
|310 |PREPARING         |wait|meas|    |
+----+------------------+----+----+----+
|320 |PREPARING_UNSTABLE|wait|    |    |
+----+------------------+----+----+----+
|330 |MOVING            |wait|    |ramp|
+----+------------------+----+----+----+
|340 |STABLIZING        |wait|    |    |
+----+------------------+----+----+----+
|350 |FINALIZING        |    |meas|    |
+----+------------------+----+----+----+
|400 |ERROR             |    |    |    |
+----+------------------+----+----+----+
|401 |UNKNOWN           |    |    |    |
+----+------------------+----+----+----+

Use cases:
  * "wait": waiting after change target before continuing measurement
  * "meas": valid measurement, useful for event mode data acquisition
  * "ramp": measurement while ramping

b) 13 Status codes, ``mode`` parameter instead of commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The ``mode`` parameter defines where to go after a change target,
e.g. preselect to stay in driven mode or go always to persistent mode.

We would need at least the following predefined meaning for mode values:

+-----------------+----+
|name             |code|
+=================+====+
|DISABLED         |   0|
+-----------------+----+
|STANDBY          |   1|
+-----------------+----+
|PREPARED         |   2|
+-----------------+----+

The mode parameter acts like below ``op_mode`` parameter. Changing the mode parameter
would trigger mode changes directly. This would then be the second exception to the rule,
that a parameter change should not lead to a BUSY state.
As long as the target mode is not reached, the status code would indicate BUSY.

With this approach, the target mode is always visible.


c) 10 status codes, parameters ``op_mode``/``op_state`` instead of commands
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

On the video meeting 2019-07-11 we decided to consider again having two
parameters set_mode/mode, which we later agreed to change to op_mode/op_state.

Proposed enum values for ``op_mode``:

+-----------------+----+
|name             |code|
+=================+====+
|DISABLED         |   0|
+-----------------+----+
|STANDBY          |   1|
+-----------------+----+
|PREPARED         |   2|
+-----------------+----+


Additional codes for ``op_state``:

+-----------------+----+
|name             |code|
+=================+====+
|initializing     | 101|
+-----------------+----+
|disabling        | 102|
+-----------------+----+
|preparing        | 103|
+-----------------+----+
|moving           | 104|
+-----------------+----+
|finalizing       | 105|
+-----------------+----+

Alternatively, we might choose negative values instead of adding 100.

Still we would need the following ``status`` values:

+-----------------------+--------------+
|status                 |use cases     |
+----+------------------+----+----+----+
|code|name              |wait|meas|ramp|
+====+==================+====+====+====+
|0   |DISABLED          |    |    |    |
+----+------------------+----+----+----+
|100 |IDLE              |    |meas|    |
+----+------------------+----+----+----+
|200 |WARN              |    |meas|    |
+----+------------------+----+----+----+
|250 |WARN_UNSTABLE     |    |    |    |
+----+------------------+----+----+----+
|300 |BUSY              |wait|    |    |
+----+------------------+----+----+----+
|310 |PREPARING         |wait|meas|    |
+----+------------------+----+----+----+
|340 |MOVING            |wait|    |ramp|
+----+------------------+----+----+----+
|350 |FINALIZING        |    |meas|    |
+----+------------------+----+----+----+
|400 |ERROR             |    |    |    |
+----+------------------+----+----+----+
|401 |UNKNOWN           |    |    |    |
+----+------------------+----+----+----+

Use cases:
  * "wait": waiting after change target before continuing measurement
  * "meas": valid measurement, useful for event mode data acquisition
  * "ramp": measurement while ramping

310 PREPARING is used for the case, when data is always stored, as in neutron
event mode. It indicates, that the value is still valid during preparing phase.
If during the preparing phase the value is unstable or invalid, a simple 300 BUSY
must be used.

350 FINALIZING is used for the case, when the value is already stable at target,
but some finalizing is still happening.


Distinction between disable and shutdown
........................................

Motivation
~~~~~~~~~~

* disable: switch off a module, it can not be used before it is again enabled
* shutdown: put into an off state, where power can be shut

If disable and shutdown are the same, it might get overcomplicated or unsafe:

Overcomplicated
~~~~~~~~~~~~~~~

In the disabled state, probably anything else than switching to enabled should
be forbidden. If after power up the default state is *disabled*, we have to do
enable the module first, which seems overcomplicated.

Unsafe
~~~~~~

After a shutdown, still a script might be running, with ``change target`` and/or
``change op_mode`` calls. Which means that after doing shutdown and before powering off,
the system might again be *enabled*.

Imagine an implementor, for safety reasons, wants to avoid this, the only remaining thing
is to block any activity after shutdown, allowing only after powering off and on, or pressing
a reset button on the hardware.

Better Solution
~~~~~~~~~~~~~~~

Implement *enabled* as an extra (bool) parameter, which is then the only way to enable a module.
The default after power up should be *enabled=true*, but shutdown sets *enabled* to *false*.
After shutdown, enabling the module must then be done explicitly, and this is much more
safe, because ``change enabled`` is not meant to be used in a script.

With this approach, status_code 0 would be named DISABLED, but op_mode 0 would be called
OFF_STATE or SHUTDOWN.

Decision
--------

For status codes see `SECoP Issue 56: Additional Busy States`_

The ``shutdown`` command is kept, but neither ``prepare`` nor ``finalize`` are specified
as predefined commands.  ``mode`` has a predefined meaning:

``"mode"``:
    A parameter of datatype enum, for selecting the operation mode of a module.
    The available operation modes can not be predefined in the specification, since
    they depend on the specific module.

    Maximum set of allowed modes:
    .. code::

        {"enum",{"members":{"DISABLED": 0, "STANDBY": 30, "PREPARED": 50}}

    The meaning of the operation modes SHOULD be described in the description.
.. DO NOT TOUCH --- following links are automatically updated by issue/makeissuelist.py
.. _`SECoP Issue 56: Additional Busy States`: issues/056%20Additional%20Busy%20States.rst
.. DO NOT TOUCH --- above links are automatically updated by issue/makeissuelist.py
