SECoP Issue 59: set_mode and mode instead of some commands (under discussion)
=============================================================================

Motivation
----------

Combining the information about flow control with other status values might not be a good idea, as:

* it is not easily extensible, if in future we need more states
* in the current model, for each target state, we need a command, where for the proposed model,
  we have a parameter set_mode instead, which takes whe same number of values
* in the current model, we do not have the information of the wanted final state. As long
  as there is only one client, this seems not important. But with multiple clients it is
  a benefit.

Proposal
--------

Introduce a mode (readonly) / set_mode (writable) parameter instead of only a mode (writable)
parameter, and skip all of prepare, finalize, shutdown.

* predefine at least the following codes for set_mode: DISABLED, LOCKED, PREPARED
* predefine at least the following codes for mode: DISABLED, LOCKED, PREPARED, INITIALIZING, PREPARING, MOVING, STABILIZE, FINALIZE
* reduce the predefined status codes to: DISABLED, IDLE, WARN, UNSTABLE, BUSY, FINALIZE, ERROR

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
* possible status codes: DISABLED, IDLE, PREPARED, BUSY, MOVING, PREPARING, MOVING, STABLIZE, FINALIZE, WARN, ERROR
* possible mode values: AUTO, MANUAL

model B
~~~~~~~

Proposed model. The status code contains only the information needed for the classical experiment.
After a change target the ECS wait before starting the measurement until the status code is either
FINALIZE, IDLE or WARN (or any substates of them).
For the special case, where one wants to measure while MOVING the set_mode/mode parameter
has to be used/examined.

* parameters: value, status, target, mode, set_mode
* commands: stop
* possible status codes: DISABLED, IDLE, BUSY, FINALIZE, WARN, ERROR
* possible set_mode values: DISABLED, LOCKED, PREPARED
* possible mode values: DISABLED, LOCKED, PREPARED, INITIALIZING, PREPARING, MOVING, STABILIZE, FINALIZE

explanations for the following tables
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The status text is choosen to use terms related to a cryomagnet, whereas the other
codes are using a more general wording.

In a row with an action, all other columns take their value before the reply message to the action is communicated.
In between the rows, always some time pass.

No `go` command is assumed. In case themodule is implemented with a `go` command, just send
a `go` after any `change target` in model A, and after any `change target` and `change set_mode`
in model B. In model B, when doing a `change target` and a `change mode` togther, only send
one `go` command.

initialize the module
~~~~~~~~~~~~~~~~~~~~~

+---------------------------+--------------------------------------------------+------------+
|model A                    |model B                                           |both        |
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|action    |mode|status code|action          |set_mode|mode        |status code|status text |
+==========+====+===========+================+========+============+===========+============+
|          |AUTO|DISABLED   |                |DISABLED|DISABLED    |DISABLED   |shut down   |
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|initialize|AUTO|BUSY       |set_mode:=LOCKED|LOCKED  |INITIALIZING|BUSY       |initializing|
+----------+----+-----------+----------------+--------+------------+-----------+------------+
|          |AUTO|IDLE       |                |LOCKED  |LOCKED      |IDLE       |zero field  |
+----------+----+-----------+----------------+--------+------------+-----------+------------+

change target (normal case)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* initial state: persistent, value=1
* wanted state: persistent, value=2
+--------------------------+----------------------------------------+-----------+
|model A                   |model B                                 |both       |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|action   |mode|status code|action   |set_mode|mode     |status code|status text|
+=========+====+===========+=========+========+=========+===========+===========+
|         |AUTO|IDLE       |         |LOCKED  |LOCKED   |IDLE       |persistent |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|target:=2|AUTO|PREPARING  |target:=2|LOCKED  |PREPARING|BUSY       |leads up   |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|PREPARING  |         |LOCKED  |PREPARING|BUSY       |heat sw    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|MOVING     |         |LOCKED  |MOVING   |BUSY       |ramping    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|STABILIZE  |         |LOCKED  |STABILIZE|BUSY       |stabilize  |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|FINALIZE   |         |LOCKED  |FINALIZE |FINALIZE   |cool sw    |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|FINALIZE   |         |LOCKED  |FINALIZE |FINALIZE   |leads down |
+---------+----+-----------+---------+--------+---------+-----------+-----------+
|         |AUTO|IDLE       |         |LOCKED  |LOCKED   |IDLE       |persistent |
+---------+----+-----------+---------+--------+---------+-----------+-----------+

prepare (go to driven state)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+------------------------+-------------------------------------------------+-------------+
|model A                 |model B                                          |both         |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|action |mode|status code|action            |set_mode|mode     |status code|status text  |
+=======+====+===========+==================+========+=========+===========+=============+
|       |AUTO|IDLE       |                  |LOCKED  |LOCKED   |IDLE       |idle         |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|prepare|AUTO|PREPARING  |set_mode:=PREPARED|PREPARED|PREPARING|BUSY       |leads up     |
+-------+----+-----------+------------------+--------+---------+-----------+-------------+
|       |AUTO|PREPARING  |                  |PREPARED|PREPARING|BUSY       |heat sw      |
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
|action      |mode  |status code|action   |set_mode|mode     |status code|status text   |
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

go to locked (persistent) state
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

+---------------------------+----------------------------------------------+-------------+
|model A                    |model B                                       |both         |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|action  |mode  |status code|action          |set_mode|mode    |status code|status text  |
+========+======+===========+================+========+========+===========+=============+
|finalize|MANUAL|FINALIZE   |set_mode:=LOCKED|LOCKED  |FINALIZE|FINALIZE   |cool sw      |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|        |MANUAL|FINALIZE   |                |LOCKED  |FINALIZE|FINALIZE   |leads down   |
+--------+------+-----------+----------------+--------+--------+-----------+-------------+
|        |MANUAL|IDLE       |                |LOCKED  |LOCKED  |IDLE       |driven stable|
+--------+------+-----------+----------------+--------+--------+-----------+-------------+

change target with predefined final state locked
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* initial state: prepared (driven), value=1
* wanted state: persistent (locked), value=2
* Remark for model B only: if the SEC Node does not accept set_mode while BUSY, wait until IDLE before changing set_mode 
* if the module has a `go` command, change mode/set_mode and target before sending `go`
+-----------------------------+-----------------------------------------------+-------------+
|model A                      |model B                                        |both         |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|action    |mode  |status code|action          |set_mode|mode     |status code|status text  |
+==========+======+===========+================+========+=========+===========+=============+
|          |MANUAL|PREPARED   |                |PREPARED|PREPARED |IDLE       |driven stable|
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|mode:=AUTO|AUTO  |PREPARED   |                |        |         |           |driven stable|
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|target:=2 |AUTO  |MOVING     |target:=2       |PREPARED|MOVING   |BUSY       |ramping      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |           |set_mode:=LOCKED|LOCKED  |MOVING   |BUSY       |ramping      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |STABILIZE  |                |LOCKED  |STABILIZE|BUSY       |stabilize    |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |FINALIZE   |                |LOCKED  |FINALIZE |FINALIZE   |cool sw      |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |FINALIZE   |                |LOCKED  |FINALIZE |FINALIZE   |leads down   |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+
|          |AUTO  |IDLE       |                |LOCKED  |LOCKED   |IDLE       |persistent   |
+----------+------+-----------+----------------+--------+---------+-----------+-------------+

change target with predefined final state prepared
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* inital state (as final state above): locked (persistent), value=2
* wanted state: prepared (driven), value=3
* Remark for model B only: if the SEC Node does not accept change target while BUSY, wait until IDLE before changing target
* if the module has a `go` command, change mode/set_mode and target before sending `go`
+-------------------------------+-------------------------------------------------+-------------+
|model A                        |model B                                          |both         |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|action      |mode  |status code|action            |set_mode|mode     |status code|status text  |
+============+======+===========+==================+========+=========+===========+=============+
|            |AUTO  |PREPARED   |                  |LOCKED  |LOCKED   |IDLE       |persistent   |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|mode:=MANUAL|MANUAL|PREPARED   |                  |        |         |           |persistent   |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|target:=1   |MANUAL|PREPARING  |set_mode:=PREPARED|PREPARED|PREPARING|BUSY       |leads up     |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |      |           |target:=3         |PREPARED|PREPARING|BUSY       |leads up     |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|PREPARING  |                  |PREPARED|PREPARING|BUSY       |heat sw      |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|MOVING     |                  |PREPARED|MOVING   |BUSY       |ramping      |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|STABILIZE  |                  |PREPARED|STABILIZE|BUSY       |stabilize    |
+------------+------+-----------+------------------+--------+---------+-----------+-------------+
|            |MANUAL|PREPARED   |                  |PREPARED|PREPARED |IDLE       |driven stable|
+------------+------+-----------+------------------+--------+---------+-----------+-------------+

shut down
~~~~~~~~~

* inital state: persistent (locked), value=2
+-------------------------+-------------------------------------------------+-----------+
|model A                  |model B                                          |both       |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|action  |mode|status code|action            |set_mode|mode     |status code|status text|
+========+====+===========+==================+========+=========+===========+===========+
|        |AUTO|IDLE       |                  |LOCKED  |LOCKED   |IDLE       |persistent |
+--------+----+-----------+------------------+--------+---------+-----------+-----------+
|shutdown|AUTO|PREPARING  |set_mode:=DISABLED|DISABLED|PREPARING|BUSY       |leads up   |
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


Conclusion
..........

The initial motivation of above proposal was:

a) splitting some information in order not to overload that status parameter.
   The number of status code is reduced from 11 to 8, which is not a lot.
   
b) the idea of implementing "slow" state parameters with a set_<state>/<state> parameter
   pair instead of a command/parameter pair a proposed in the 2019-05-20 meeting at MLZ.
   As we do prefer to have module instead of "slow" state parameters, we do not have
   to introduce a new concept here.

If we come back to the ideas from the MLZ meeting, but without the information about
accepting new commands, this would lead to the following list:


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
  
However, we have to decide how to trigger mode changes:

a) Commands: shutdown, prepare, finalize
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We may still need a 'mode' parameter, in order to define where to go after a change target,
e.g. preselect to stay in driven mode or go always to persistent mode.

b) Only a mode parameter
~~~~~~~~~~~~~~~~~~~~~~~~

The mode parameter acts like above 'set_mode parameter'. Changing the mode parameter
would trigger mode changes directly. This would then the second exception to the rule,
that a parameter change should not lead to a BUSY state.

The advantage of approach (b) is, that the target mode is always visible.

We would need at least the following predefined meaning for mode values:

  * DISABLE(D)  = 0 ?
  * LOCK(ED)    = 1 ?
  * PREPARE(D)  = 2 ?


c) mode/mode_state
~~~~~~~~~~~~~~~~~~

On the video meeting 2019-07-11 we decided to consider again having two
parameter set_mode/mode. Markus proposes to change them to mode/mode_state -
better proposals for naming are welcome. 'status' is already used, 'state'
is to close to 'status', but 'mode' alone seems not suitable for somthing, which
might have a transitional state. Other alternative names for 'set_mode': 'target_mode'
of 'mode_target'.

Proposed enum values for (set_)mode:

+-----------------+----+
|name             |code|
+=================+====+
|disabled         |   0|
+-----------------+----+
|idle (or locked?)|   1|
+-----------------+----+
|prepared         |   2|
+-----------------+----+


Additonal codes for mode(_state):

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


Alternatively, we could choose negative values instead of adding 100.

Still we would need the following status values:

+-----------------------+---------+
|status                 |use cases|
+----+------------------+----+----+
|code|name              |wait|meas|
+====+==================+====+====+
|0   |DISABLED          |    |    |
+----+------------------+----+----+
|100 |IDLE              |    |meas|
+----+------------------+----+----+
|200 |WARN              |    |meas|
+----+------------------+----+----+
|250 |WARN_UNSTABLE     |    |    |
+----+------------------+----+----+
|300 |BUSY              |wait|    |
+----+------------------+----+----+
|310 |PREPARING         |wait|meas|
+----+------------------+----+----+
|350 |FINALIZING        |    |meas|
+----+------------------+----+----+
|400 |ERROR             |    |    |
+----+------------------+----+----+
|401 |UNKNOWN           |    |    |
+----+------------------+----+----+

310 PREPARING is used for the case, when data is always stored, as in neutron
event mode. It indicates, that the value is still valid during preparing phase.
If during the preparing phase the value is unstable or invalid, a simple 300 BUSY
must be used.

350 FINALIZING is used for the case, when the value is already stable at target,
but some finaling is still happening.

The 'MOVING' status is no longer reflected in the status, but must be dereived from
mode(_state).
