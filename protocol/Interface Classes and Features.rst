Interface Classes and Features
==============================

Interface Classes
-----------------

Readable
~~~~~~~~

.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - value
      - *any*
      -
      -
      - main value    

    * - status
      - ["enum", {"idle": 100, "warn": 200, "error": 400}]
      -
      -
      - state of the module. additional substati may be defined by adding a
        number between 1 and 99
 
      
Writable
~~~~~~~~

inherits *value* and *status* from *Readable*

.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - target
      - the same datatype as *value*
      - $
      -
      - target value

      
Drivable
~~~~~~~~

inherits *value*, and *target* from *Writable*, *status* is extended

.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - status
      - ["enum", {"idle": 100, "warn": 200, "busy": 300, "error": 400}]
      -
      -
      - state of the module

    * - stop
      - ["command"]
      - 
      -
      - stops driving (has only an effect when the status is 'busy',
        stopping is finished when the status is no more 'busy')


Magnet
~~~~~~

inherits *value*, *target*, *status" and *stop* from *Drivable*

may have features *HasPersistence*, *HasWindow*, *HasTimeout*

.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - value
      - ["float", <min value>, <max value>]
      - T
      - 
      - nominal magnetic field 

    * - current_in_leads
      - ["float"]
      - A or T 
      -
      - 

Temperature Controller
~~~~~~~~~~~~~~~~~~~~~~

to be done

Features
--------

HasPid
~~~~~~
    
.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - use_pid
      - ["enum", {"open_loop": 0, "pid_control": 1}]
      -
      -
      - use pid control    

    * - p
      - ["float", 0]
      -
      -
      - proportional part of the regulation    

    * - i
      - ["float", 0]
      -
      - optional
      - integral part of the regulation    

    * - d
      - ["float", 0]
      -
      - optional
      - derivative part of the regulation 
      
    * - base_output
      - ["float"]
      - 
      - optional
      - output offset  

    * - pid
      - ["struct", {"p": ["float", 0], "i": ["float", 0], "d": ["float", 0],
        "base_output": ["float", 0]}]
      - 
      - optional
      - struct may be extended with custom elements
    
    * - output
      - ["float"]
      -
      - optional
      - output for pid control

Notes
   * implementors should either use p,i,d or pid, but ECS must be handle both cases
   * if both p,i,d and pid are implemented, it MUST NOT matter which one gets a change, the final result should be the same
   * if there are additional custom parameters with the same name as an element of the struct, the above applies
   * should still be in the same group, though
   * if extra elements are implemented in the pid struct they MUST BE properly described in the description of the pid parameter
    
   * *base_output was introduced here because the LakeShore controllers have it. In principle, for pure
     PID control it is not needed, but it helps for faster control when changing the setpoint, and it may be
     used when "i" is zero, to compensate for the missing integral part.
     We should rediscuss if base_output should not be treated as a custom parameter / custom element,
     as it is quite specific*


HasPidTable
~~~~~~~~~~~

.. list-table::
    :widths: 20 30 10 10 30
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description

    * - use_pid_table
      - ["enum", {"fixed_pid": 0, "use_pid_table": 1}]
      - 
      - 
      - use pid table

    * - pid_table
      - ["array", ["struct", {"p": ["float", 0], "i": ["float", 0], "d": ["float", 0]}]]
      - 
      - optional
      - pid table (struct may include additional custom elements)

*Note: "base_output" is omitted here*


HasPersistence
~~~~~~~~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - persistent_mode
      - ["enum", {"off": 0, "on": 1}]
      - 
      - 
      - use persistent mode
      
    * - is_persistent
      - ["bool"]
      - 
      - optional
      - current state of persistence
      
    * - stored_value
      - ["float"] (or something else, but the same as the modules value datatype
      - $
      - readonly, optional
      - current persistence value, often used as the modules value
      
    * - driven_value
      - ["float"] (or something else, but the same as the modules value datatype
      - $
      - readonly, optional
      - current persistence value, often used as the modules value.


*To be discussed: should the enum values of the status to be extended with
"decoupled" (substate of "idle"), "coupling", "coupled", "decoupling" (substate of "busy")?*


HasTolerance
~~~~~~~~~~~~

detects IDLE status by checking if the value lies in a given window:
tolerance is the maximum allowed deviation from target, value must lie in this interval
for at least "time_window" seconds.


.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - tolerance
      - ["float", 0]
      - $
      -
      - tolerance, half height of the window for convergence criterium

    * - time_window
      - ["float", 0]
      - s
      - optional
      - length of the time window for convergence criterium


HasTimeout
~~~~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - timeout
      - ["float", 0]
      - s
      -
      - timeout for driving


HasRamp
~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - ramp
      - ["float", 0]
      - $/min
      - 
      - speed of movement
      
    * - use_ramp
      - ["enum", {"disable_ramp": 0, "use_ramp": 1}]
      - 
      - optional
      - use the ramping of the setpoint, or jump
      
    * - setpoint
      - ["float"]
      - $
      - readonly
      - currently active setpoint
      
HasSpeed
~~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - speed
      - ["float", 0]
      - $/s
      - 
      - speed of movement

HasAccel
~~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - accel
      - ["float", 0]
      - $/s^2
      - 
      - acceleration of movement

    * - decel
      - ["float", 0]
      - $/s^2
      - optional
      - deceleration of movement

HasMotorCurrents
~~~~~~~~~~~~~~~~

.. list-table::
    :widths: 20 20 10 10 40
    :header-rows: 1
    :stub-columns: 1

    * - Accessibles
      - Datatype
      - Unit
      - Readonly, Optional
      - Description
      
    * - movecurrent
      - ["float", 0]
      - 
      - 
      - current while moving

    * - decel
      - ["float", 0]
      - 
      -
      - current while idle
