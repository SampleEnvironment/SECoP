SECoP Issue 27: Proposals for Interface Classes (under discussion)
==================================================================


Motivation
----------
So far only 3 base classes are specified.
Those cover the absolute bare minimum, yet they are not sufficient for full blown OOP ECS systems.

In this Issue, proposals for additional interface classes are to be collected.

Proposal
--------
Add following interface classes:

:note: these are proposed by MLZ. Just saying so, before somebody complains.
       So far nobody else has made a proposal for over a year, so these are now proposed.

Sensor
~~~~~~
This interface class supposed to represent a Sensor for some physical quantity (pressure, temperature, etc..).
It is derived from ``Readable``, has a numeric main value and a proper (i.e. non-empty) ``unit``.

added accessibles:

calibration:
  mandatory, a string containing a reference to the calibration of the sensor. May also contain 'factory' to indicate a factory calibration.

adjust:
  optional, a command with a numeric argument and no result, essentially adjusting the current reading to the given value.
  (i.e. if a sensor shows 10.2 mm and gets an adjust(10), it will then read 10.0 mm).

:remark from Markus:
    I would stringly recommend to have an 'offset' or 'zero' parameter in addition. This makes changing offset
    reproducible. It might not be needed to define the exact meaning of offset, especially the sign, but
    this way it would be easy to change back to a previous value reproducibly.

Actuator
~~~~~~~~

This interface class supposed to represent an actuating/regulating module for some physical quantity (pressure, temperature, etc..).
It is derived from ``Drivable``, has a numeric main value + ``target`` and a proper (i.e. non-empty) ``unit``.

added accessibles:

ramp:
  mandatory*, a numeric value representing the wanted speed of change in main units per minute.

speed:
  mandatory*, a numeirc value representing the wanted speed of change in main units per second.

adjust:
  optional, a command with a numeric argument and no result, essentially adjusting the current reading to the given value.
  (i.e. if a sensor shows 10.2 mm and gets an adjust(10), it will then read 10.0 mm).

\*either ``ramp`` or ``speed`` are mandatory. Both may be implemented simultaneously, but honor Issue 31!


PIDController
~~~~~~~~~~~~~
A ``PIDController`` is an ``Actuator`` with a PID-loop regulating some output.

added accessibles:

mode:
    mandatory, an enum indicating the current operation mode: 'open-loop', 'pid-control', 'ramp'

setpoint:
    mandatory, readonly value, represents the currently active setpoint (which may be ramping towards the target)
    for the regulation loop

output:
    mandatory, represents the current output value (the heater output power for a temperaturecontroller).
    The unit indicates absolute or relative units ('W'/... or '%' respectively).
    May be writable, if so, the mode should switch to open-loop if written.

maxoutput:
    optional, an enum indicating the currently selected output range.

    :note: use case for a 'one-of' datatype. to be proposed.

pid:
    mandatory, a struct with the members 'p', 'i' and 'd'.
    'i' and 'd' may be optional.
    note: it would be sensible to allow partial structs in ``update`` messages to only change one of p,i or d.
    See Issue 35: Partial structs.


Communicator
~~~~~~~~~~~~
A ``Communicator`` is a base class for modules for communication to the underlying hardware.
It is used, if the SEC-node is utilizing a stacked approach and not just representing a top-level view.
It may still restrict access to the modules derived from this class.

This is the only interface_class which has no ``value`` accessible!

:proposal: move this class into the specification ASAP, so that custom implementations can flag a communication module as such!


StringIO
~~~~~~~~
Used for string based communication. subclass of ``Communicator``.

added accessibles:

communicate:
    mandatory, command with a String as argument and as result.

    Sends a message, waits, and returns the received answer.

    This command is a combination of writeLine and readLine, with integrated blocking in between.

flush:
    optional, command with neither argument nor result.

    Flushes the output data to the connection.

read:
    mandatory, command with an integer as argument and a string as result.

    Reads up to n chars (the input argument) from the input buffer and returns them as string.

write:
    mandatory, command with a string as argument and an integer as result.

    Writes a raw string to the connection (without appending/prepending anything).
    returns the number of **bytes** transferred.

writeLine:
    mandatory, command with a string as argument and an integer as result.

    Writes the given line to the connection.

    In addition it prepends “startOfLine” and appends “endOfLine”.

readLine:
    mandatory, command with no argument and a string as result.

    Reads a full line from the input buffer and returns it (excluding “startOfLine” and “endOfLine”).

multiCommunicate:
    mandatory, command with an array of tuples(double,string) as argument and an array of strings as result.

    The input strings are the messages to be sent.
    The double value specifies how to send the messages:

    - if the value is negative or zero, the message is sent with Communicate and a reply is expected
    - if the value is positive, the message is sent with WriteLine (i.e. no reply is expected)
    - after each message, a sleep of the absolute value of the value (in seconds) is done

    For example, the call “multiCommunicate([[0.1,‘CHAN 1’],[0,‘MEAS?’]])” would send “CHAN 1” with WriteLine, wait 0.1 seconds, and then do a Communicate with “MEAS?”, and return a list with one element, the reply of 'MEAS?'.

    This does multiple communicates but blocks the module until all communicates are done.

availableChars:
    optional, readonly integer value specifying the number of bytes in the input buffer (,coming from the hardware).

availableLines:
    optional, readonly integer value of the number of lines in the input buffer.

communicationTimeout:
    optional, a positive double value.

    The timeout for the communication between SEC-node and hardware (in seconds).

endOfLine:
    mandatory, possibly readonly, a string with the current the 'end of line' char sequence.
    may also be a tuple of strings:(value_for_sending, value_for_receiving)

startOfLine:
    mandatory, possibly readonly, a string with the current the 'start of line' char sequence.
    may also be a tuple of strings:(value_for_sending, value_for_receiving)

echo:
    mandatory, possibly readonly, a bool indicating wheather the hardware echos back what it receives or not.


BinaryIO
~~~~~~~~
Used to exchange blobs of predefined lengths with hardware. subclass of ``Communicator``.

added accessibles:

binaryCommunicate:
    mandatory, command with a tuple(expected_reply_len as integer, blob) as argument and a blob as result.

    Sends a message, waits, and returns the received answer.

    After sending the message, the module will read bytes until either the expected number of characters is reached,
    or the “communicationTimeout” is expired.
    If some bytes have arrived when timeout hits, they are returned.
    If none have arrived, an error ``CommunicationFailed`` is returned.

binaryRead:
    mandatory, command with an integer as argument and a blob as result.

    Reads up to n bytes (the input argument) from the input buffer.

binaryWrite:
    mandatory, command with a blob as argument and an integer as result.

    Writes a number of bytes to the connection.

availableChars:
    optional, readonly integer of the currently seen number of bytes in the buffer.

    Note: Even if the number of bytes does not change between two calls, the contents of the data may still have been changed.

communicationTimeout:
    optional, a positive double value.

    The timeout for the communication between SEC-node and hardware (in seconds).

echo:
    mandatory, possibly readonly, a bool indicating wheather the hardware echos back what it receives or not.


BusIO
~~~~~
Used for Profibus/Modbus/other register adressable protocols.

defines several commands which follow the scheme: action + scheme + datatype + mulitplier where

* action = "read" or "write"
* scheme = "Input" or "Output" or "" (if no distinction)
* datatype:

  ========== =============
   datatype   description
  ========== =============
    Bit       single bit, value 0 or 1
    Byte      8 bits, value 0..255
    Word      16 bits, value 0..65535
    DWord     32 bits, value 0..4294967295
    QWord     64 bits, value 0..18446744073709551615
    Float     32 bits, IEE754 binary32, value +/-1.18 × 10−38..+/-3.402823 × 1038
    Double    64 bits, IEE754 binary64, value +/-2.2250738585072014e-308..+/-1.7976931348623157e+308
  ========== =============

  note: all datatypes are **unsigned**.

* multiplier = "" for single qunatities, else "s"

The argument is always the address to read from (for read*) or
a tuple with the address as first element and the data to be written as the second element.
If multiple data is to be written, an array is used as the second element.
If multiple data is to be read, an array is used for the result, else the value directly.

for each supported datatype which is bigger then the native datatype of the addressable hardware,
a '*order' accessible is to be defined as an enum(Big-endian, Little-endian).
A mixed-order is not foreseen, but could be implemented as a further enum value, if needed.

i.e. for Modbus (where the addressable elements are 'registers' of 16 bit, or 'words'),
a ``readInputFloat`` command would require the definition of a ``floatorder`` enum.
These enums may not be exported via SECoP (i.e. in SECoP they are optional).

An ``BusIO`` module may also have additional accessibles:

slaveId:
    optional, an integer specifying the slave-id to use in communications

transaction:
    optional, a command which initiates the data-exchange with the hardware, if that isn't done with the read/write commands automatically.


Alternative list of proposed interface classes (outdated syntax!)
-----------------------------------------------------------------

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

******

Discussion
----------
topic raised on several discussions.
not discussed in present form.

:note: the above is a first workable proposal.

:remark from Markus:
    Instead of Sensor, Actuator, PIDController we should have features HasCalibration, HasOffset,
    HasRamp, HasPID as there are no interdependencies.


video conference 2018-11-07
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Klaus investigate the usefulness of the proposed classes.

``Sensor``, ``Actuator``, ``PIDController`` and ``Communicator`` are already good candidates for inclusion
into the next version of the spec. The others are too specific and may be implemented as custom classes (prefix class name with '_' !)


Decision:
 - keep as 'under discussion'


