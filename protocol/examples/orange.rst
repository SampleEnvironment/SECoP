Example of a wet (liquid helium cooled) cryostat for SECoP
==========================================================

:authors:
    Klaus Kiefer;
    Bastian Klemke;
    Lutz Rossa

:Version: 0.14 of 2022-09-26

Introduction
------------

| The following draft is demonstrating an example SECoP implementation of a
  wet cryostat. An "Orange Cryostat" is a good example, for details of working
  principle and history we refer to ILL_ and AS_ .

This SECoP interface of a wet cryostat is equipped with a variable temperature
insert (VTI) and a corresponding cold valve (here named: needle valve or
simply "NV").

We show here an implementation with three different levels of complexity:

        user, advanced and expert.

The user and advanced level implementations are grouped together in one
descriptive JSON_ indicated by the visibility "user" and "advanced".
The expert level implementation is in a separate descriptive JSON_,
because of write access.

Some of the custom parameters (starting underscore in name) are still in
discussion and might be defined in a future SECoP version, which will result
in a change in this document.

The SECoP specification allows optional parameters and commands. We recommend
for convenience and more clarity to implement the specified accessibles (see
below), as if they were declared mandatory.

user level
----------

``temperature_reg`` *(drivable)*

    This is the regulation temperature of the VTI.

    standard parameters and commands:
        ``value``, ``status``, ``target`` and ``stop``

    properties:
        ``meaning`` with ``temperature_regulation``

``temperature_sample`` *(readable)*

    Due to the fact that the sample is located in the sample space with
    surrounding helium exchange gas, the sample temperature in most
    cases is weakly coupled to the regulation temperature of the VTI.
    Therefore, a separate module sample temperature is used.

    standard parameters:
        ``value``, ``status`` (``disabled``, ``idle``, ``warn``, ``error``)

    properties:
        ``meaning`` with ``temperature``

advanced level
--------------

*Please look also for comments in the "user level" section.*

``temperature_reg`` *(drivable)*

    standard parameters and commands:
        ``value``, ``status``, ``target`` and ``stop``

    ``_calibration_table`` *(static)*
        Each temperature sensor has a calibration table to derive the
        temperature from the sensor value (e.g. resistance of a resistance
        thermometer or voltage of a diode). This datatype is not standardized,
        e.g. representing a calibration table, calibration polynomial.

    ``ctrlpars`` *(readonly)*
        ``struct`` [``p``, ``i``, ``d``, ``_heaterrange_value``, ``_nv_pressure``]

        For a suitable (temperature) regulation in most cases a "PID
        regulation" is used. Please see `SECoP Issue 67`_ .

        For details of control theory, we refer to ControlTheory_ and PIDcontroller_ .
        Often used are temperature controller from Lake Shore (LS_) and
        Oxford Instruments (OI). No link to a OI webpage with PID available.

        For a stable regulation at different temperatures (e.g. near room
        temperature or near base temperature of cryostat), it is recommended
        to adapt the maximum heater output (heater range) and the position or
        pressure of the needle valve. Therefore, we extended the ``ctrlpars``
        with ``_heaterrange_value`` (see ``power_reg:_heaterrange_value``) and
        ``_nv_pressure`` (see ``_pressure_vti``).

    properties:
        ``meaning`` with ``temperature_regulation``

``power_reg`` *(readable)*

    Heater power applied to temperature regulation control loop.

    standard parameters:
        ``value``, ``status``

    ``_heaterrange_enum`` *(readonly)*
        *influences* [``power_reg:_heaterrange_value``]

    ``_heaterrange_value`` *(readonly)*
        *influences* [``power_reg:_heaterrange_enum``]

    In most cases, for stable regulation the heater range (maximum
    heater output) has to be adapted (see ``temperature_reg:ctrlpars``).

    Due to the fact that many temperature controllers only have distinct
    heater ranges to select, we introduce here an enumeration and a value
    which are depending on each other indicated by "influences" see
    `SECoP Issue 63`_ and `SECoP Issue 65`_ .

``temperature_sample`` *(readable)*

    standard parameters:
        ``value``, ``status``

    ``_calibration_table`` *(static)*

    properties:
        ``meaning`` with ``temperature``

``temperature_additional_sensor_x`` *(readable)* *[x=1..number-of-sensors]*

    Depending on the complexity of the performed experiments, it might
    be necessary to place more temperature sensors.

    standard parameters:
        ``value``, ``status``

    ``_calibration_table`` *(static)*

``pressure_samplespace`` *(readable)*

    Pressure of exchange gas in the sample space.

    standard parameters:
        ``value``, ``status``

``pressure_vti`` *(readable)*

    The pressure of the VTI (together with the pumping speed of the
    used pump) is linked to the cooling power from the evaporation of
    liquid helium in the variable temperature insert. Therefore, it is
    implemented in the ``temperature_reg:ctrlpars``.

    standard parameters:
        ``value``, ``status``

``position_nv`` *(readable)*

    The opening position of the needle valve is directly linked to the
    pressure of the VTI (see ``pressure_vti``).

    standard parameters:
        ``value``, ``status``

``heliumlevel`` *(readable)*

    liquid helium filling level of the cryostat

    standard parameters:
        ``value``, ``status``

``nitrogenlevel`` *(readable)*

    liquid nitrogen filling level of the cryostat

    standard parameters:
        ``value``, ``status``

expert level
------------

In the "expert" level not only the regulation temperature is a drivable
but also the regulation power, the needle valve pressure and the needle
valve position are drivables. All of those drivables can be used to change
the temperature. Setting the target value of one of those modules will
activate the corresponding control and might deactivate one or more of the
other controllers (see target "influences" of all four modules).

The last target value which was set is defining which control is active.
Therefore, a mechanism is required to indicated which of the controls
is active and which module is controlled by another module (see decision
of `SECoP Issue 65`_).

The ``control_active`` parameter (``bool``) indicates, if the value (e.g.
temperature) will be influenced (within the physical limit) by the target.
If ``control_active = True`` in best case the target will be reached.

How this mechanism works can be seen in an illustrated example at the end
of this document.

*Please look also for comments in the "user level" and "advanced
level" sections.*

``temperature_reg`` *(drivable)* --> *temperature regulation module*

    standard parameters and commands:
        ``value``, ``status``, ``stop``, ``ramp``, ``setpoint``,
        ``time_to_target``, ``go``, ``shutdown``, ``hold``,
        ``clear_error``, ``target``

        **optional** are: ``go``, ``shutdown``

    ``target``
        *influences* [``power_reg:controlled_by``, ``pressure_vti:controlled_by``, ``self:control_active``]

    ``_sensor_value`` *(readonly)*
        | ``struct`` [e.g. temperature, resistance]
        | representing the temperature and the corresponding sensor
          value e.g. resistance or voltage.

    ``_calibration_table`` *(static)*

    ``ctrlpars`` *(not readonly)*
        ``struct`` [``p``, ``i``, ``d``, ``_heaterrange_value``, ``_nv_pressure``]

    ``control_active`` *(readonly)*
        | **mandatory** bool
        | see above and `SECoP Issue 65`_

    ``_automatic_nv_pressure_mode`` *(not readonly)*
        | ``enabled`` or ``disabled``
        | *influences* [``pressure_vti:controlled_by``]

        The needle valve can be operated in automatic mode, which
        means that the needle valve pressure from the ``temperature_reg:ctrlpars``
        is used. If for some reason, a stable needle valve pressure is
        needed (e.g. to optimize the liquid helium consumption), the
        automatic needle valve pressure mode can be disabled, as well.

        If disabled, there is no control from ``temperature_reg`` to module
        ``pressure_vti``.

    properties:
        ``meaning`` with ``temperature_regulation``

``power_reg`` *(drivable)*

    standard parameters and commands:
        ``value``, ``status``, ``stop``, ``ramp``, ``setpoint``,
        ``time_to_target``, ``go``, ``shutdown``, ``hold``,
        ``clear_error``, ``target``

        **optional** are: ``go``, ``shutdown``

    ``target``
            *influences* [``temperature_reg:control_active``, ``self:controlled_by``]

    ``_heaterrange_enum`` *(not readonly)*
        *influences* [``power_reg:_heaterrange_value``]

    ``_heaterrange_value`` *(not readonly)*
        *influences* [``power_reg:_heaterrange_enum``]

    ``controlled_by`` *(readonly)*
        | **mandatory** enum
        | [``self``, ``temperature_reg``]

        Due to the fact that only one module can be in charge of the
        control the ``controlled_by`` parameter is used. See above and
        `SECoP Issue 65`_.

``temperature_sample`` *(readable)*

    standard parameters:
        ``value``, ``status``

    ``_sensor_value`` *(readonly)*
        ``struct`` [e.g. temperature, resistance]

    ``_calibration_table`` *(static)*

    properties:
        ``meaning`` with ``temperature``

``temperature_additional_sensor_x`` *(readable)* *[x=1..number-of-sensors]*

    standard parameters:
        ``value``, ``status``

    ``_sensor_value`` *(readonly)*
        ``struct`` [e.g. temperature, resistance]

    ``_calibration_table`` *(static)*

``pressure_samplespace`` *(drivable)*

    If the cryostat is equipped with an automatic valve to flush and
    purge the sample space, also the pressure of the sample space can
    be a drivable.

    standard parameters and commands:
        ``value``, ``status``, ``target``, ``stop``

    **mandatory** are: all

``pressure_vti`` *(drivable)*

    standard parameters and commands:
        ``value``, ``status``, ``target``, ``stop``

    **mandatory** are: all

    ``target``
        *influences* [``temperature_reg:_automatic_nv_pressure_mode``, ``self:controlled_by``]

    ``controlled_by`` *(readonly)*
        | **mandatory** enum
        | [``self``, ``temperature_reg``]
        | See above and `SECoP Issue 65`_.

    ``control_active`` *(readonly)*
        | **mandatory** bool
        | See above and `SECoP Issue 65`_.

``position_nv`` *(drivable)*

    standard parameters and commands:
        ``value``, ``status``, ``target``, ``stop``

    **mandatory** are: all

    ``target``
        *influences* [``pressure_vti:control_active``, ``self:controlled_by``]

    ``controlled_by`` *(readonly)*
        | **mandatory** enum
        | [``self``, ``pressure_vti``]
        | See above and `SECoP Issue 65`_.

``heliumlevel`` *(readable)*

    standard parameters:
        ``value``, ``status``

``nitrogenlevel`` *(readable)*

    standard parameters:
        ``value``, ``status``

Control flow
------------

*Remark:*
    | Solid lines depict that a module passes its internal setpoint
      to the target of the depending module.
    | Dashed lines depict the physical connection.

``temperature_reg`` is in charge:
#################################

| ``temperature_reg:target`` was set to a valid new value
| and ``temperature_reg:_automatic_nv_pressure_mode`` is ``True``.

    .. image:: orange_control1.png

``temperature_reg`` is in charge, but not ``pressure_vti``:
###########################################################

| ``temperature_reg:_automatic_nv_pressure_mode`` is ``False``
| or ``pressure_vti:target`` was set to a valid new value.

    .. image:: orange_control2.png

``temperature_reg`` is in not charge, but ``power_reg`` and ``pressure_vti``:
#############################################################################

| ``power_reg:target`` was set to a valid new value and
| ``temperature_reg:_automatic_nv_pressure_mode`` is ``False`` or
| previously ``pressure_vti:target`` was set to a valid new value.

The control connection ``temperature_reg:control_active`` to
``power_reg:controlled_by`` (or ``pressure_vti:controlled_by``) disappears.
The parameter ``temperature_reg:control_active`` goes to ``False``.

The parameters ``power_reg:controlled_by`` and ``pressure_vti:controlled_by``
go automatically to ``self``. If any ``???:target`` parameter changed, the
value updates have to be sent before the reply of the change command.

    .. image:: orange_control3.png

Only ``power_reg`` and ``position_nv`` are in charge:
#####################################################

``position_nv:target`` was set to a valid new value.

The module ``pressure_vti`` is no longer controlling ``position_nv``,
``pressure_vti:control_active`` goes to ``False`` and
``position_nv:controlled_by`` goes to ``self``. This also propagates the
break of a possible control connection of ``temperature_reg`` to
``pressure_vti`` (see above).

Additionally and depending if the parameter
``temperature_reg:_automatic_nv_pressure_mode`` is ``True``, the
``temperature_reg:control_active`` has to go to ``False`` too and this
also breaks any control connection to ``power_reg`` (see above).

Same happens, setting ``power_reg:target`` or ``pressure_vti:target`` above
with consequences in a chain to ``pressure_vti`` to ``temperature_reg`` to
``power_reg``.

    .. image:: orange_control4.png

.. _ILL: https://www.ill.eu/users/support-labs-infrastructure/sample-environment/services-for-advanced-neutron-environments/history/cryogenics/orange-cryostats/
.. _AS: http://www.asscientific.com/products/cryostats.html
.. _JSON: https://www.json.org/
.. _ControlTheory: https://en.wikipedia.org/wiki/Control_theory
.. _PIDcontroller: https://en.wikipedia.org/wiki/PID_controller
.. _LS: https://www.lakeshore.com/docs/default-source/temperature-catalog/lstc_appendixf_l.pdf
.. _`SECoP Issue 63`: https://github.com/SampleEnvironment/SECoP/blob/master/protocol/issues/063%20enumeration%20of%20floating%20point%20values.rst
.. _`SECoP Issue 65`: https://github.com/SampleEnvironment/SECoP/blob/master/protocol/issues/065%20handling%20of%20coupled%20modules.rst
.. _`SECoP Issue 67`: https://github.com/SampleEnvironment/SECoP/blob/master/protocol/issues/067%20pid%20control%20parameters.rst
