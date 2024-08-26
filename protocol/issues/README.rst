SECoP Issues
============

The idea behind SECoP Issues is to document properly what was proposed,
what was discussed, what was decided and why it was decided.

An issue might take different states:

(u) unspecified
---------------

a vague idea, but no proposal written down yet

(p) proposed
------------

A proposal should contain the motivation. As long as nobody else
joins into a discussion, the state remains *proposed*

(d) under discussion
--------------------

This state is kept until there is a decision taken.

(f) finalizing
--------------

This state is kept until the specification change is agreed.

( ) closed
----------

After a decision the state is *closed*. The issue is not deleted,
even if the decision was to not follow further the proposal.
This is helpful if somebody later rises a similar issue.
However, it should be possible to reopen an issue, if new
arguments arise.

Remark:
-------

At the meeting in Lund (13th June 2018), it was agreed not to follow the proposal
of creating a new state "extensible" for an issue containing an extensible
list. Instead, an new issue should be created, containing the added elements.
A full actual list should be added each time.


Issues List
===========

.. table::

    ===== =======
    \     `SECoP Issue 1: About SECoP Issues`_
    \     `SECoP Issue 2: Equipment ID in Describing Message`_
    \     `SECoP Issue 3: Timestamp Format`_
    \     `SECoP Issue 4: The Timeout SEC Node Property`_
    \     `SECoP Issue 5: Definition of the term 'Qualifier'`_
    \     `SECoP Issue 6: Keep Alive`_
    \     `SECoP Issue 7: Time Synchronization`_
    \     `SECoP Issue 8: Groups and Hierarchy`_
    \     `SECoP Issue 9: Module Meaning`_
    \     `SECoP Issue 10: Character set for Names`_
    \     `SECoP Issue 11: Default parameters 'value' and 'target'`_
    \     `SECoP Issue 12: JSON values vs documents`_
    \     `SECoP Issue 13: Access Control`_
    u     `SECoP Issue 14: Synchonisation of Hardware Modules on Different SEC Nodes`_
    \     `SECoP Issue 15: Support for Start/Stop/Pause/Shutdown Commands`_
    \     `SECoP Issue 16: wanted_target Parameter`_
    \     `SECoP Issue 17: Collect Use Cases for 'starting'`_
    d     `SECoP Issue 18: Interface Classes`_
    \     `SECoP Issue 19: Set SEC Node clock over SECoP`_
    u     `SECoP Issue 20: PID tables`_
    \     `SECoP Issue 21: Usage of JSON null`_
    u     `SECoP Issue 22: Enable Module instead of Shutdown Command`_
    \     `SECoP Issue 23: Adjust Datatypes`_
    \     `SECoP Issue 24: Mix Commands within Parameters in the Description`_
    \     `SECoP Issue 25: Pong Format`_
    \     `SECoP Issue 26: More Module Meanings`_
    d     `SECoP Issue 27: Proposals for Interface Classes`_
    \     `SECoP Issue 28: Clarify buffering mechanism`_
    \     `SECoP Issue 29: New messages for buffering`_
    \     `SECoP Issue 30: Clarify Message parsing`_
    \     `SECoP Issue 31: Clarify timing requirements of status updates`_
    \     `SECoP Issue 32: Clarify specification using RFC-style definitions`_
    \     `SECoP Issue 33: Include section about security and access control`_
    \     `SECoP Issue 34: Clarify 'do' message`_
    \     `SECoP Issue 35: Partial Structs`_
    \     `SECoP Issue 36: Dynamic units`_
    \     `SECoP Issue 37: Clarification of status`_
    \     `SECoP Issue 38: Extension mechanisms`_
    p     `SECoP Issue 39: Use cases`_
    p     `SECoP Issue 40: Finalise Specification`_
    \     `SECoP Issue 41: Handling of JSON-numbers`_
    \     `SECoP Issue 42: Requirements of datatypes`_
    \     `SECoP Issue 43: Parameters and units`_
    \     `SECoP Issue 44: Scaled integers`_
    \     `SECoP Issue 45: Async error updates`_
    \     `SECoP Issue 46: remote logging`_
    \     `SECoP Issue 47: Error classes`_
    \     `SECoP Issue 48: mode parameter`_
    \     `SECoP Issue 49: Precision of Floating Point Values`_
    \     `SECoP Issue 50: Reserved Names`_
    \     `SECoP Issue 51: No Restrictions on Datatype Nesting`_
    \     `SECoP Issue 52: Include Some Properties into Datatype`_
    \     `SECoP Issue 53: Custom Status Values`_
    \     `SECoP Issue 54: Splitting _update_ into two Message Types`_
    \     `SECoP Issue 55: Reformat Datatype description`_
    \     `SECoP Issue 56: Additional Busy States`_
    \     `SECoP Issue 58: use JSON key, value for describe message`_
    \     `SECoP Issue 59: set_mode and mode instead of some commands`_
    u     `SECoP Issue 60: enhancements to the documentation`_
    \     `SECoP Issue 61: new predefined property implementation`_
    u     `SECoP Issue 62: naming convention for related parameters`_
    \     `SECoP Issue 63: enumeration of floating point values`_
    \     `SECoP Issue 64: reading multiple parameters simultaneously`_
    \     `SECoP Issue 65: handling of coupled (sub)modules`_
    \     `SECoP Issue 66: force re-connect`_
    f     `SECoP Issue 67: pid control parameters`_
    \     `SECoP Issue 68: transaction of multiple commands`_
    f     `SECoP Issue 69: optional structs by default`_
    u     `SECoP Issue 70: message is a JSON object`_
    \     `SECoP Issue 71: accessing sub items of parameters`_
    \     `SECoP Issue 72: features`_
    p     `SECoP Issue 73: HasLimits and HasOffset`_
    u     `SECoP Issue 74: Standardization of units`_
    p     `SECoP Issue 75: New messages check and checked`_
    p     `SECoP Issue 76: Interface for Measurable hardware`_
    p     `SECoP Issue 77: predefined parameter name prefixes`_
    p     `SECoP Issue 78: Interacting Modules - use case power supply`_
    \     `SECoP Issue 79: hints for readonly access`_
    p     `SECoP Issue 80: issues with software ramp`_
    p     `SECoP Issue 81: dynamic descriptive data`_
    p     `SECoP Issue 82: Feature HasRamp`_
    ===== =======

.. _`SECoP Issue 1: About SECoP Issues`: 001%20About%20SECoP%20Issues.rst
.. _`SECoP Issue 2: Equipment ID in Describing Message`: 002%20Equipment%20ID%20in%20Describing%20Message.rst
.. _`SECoP Issue 3: Timestamp Format`: 003%20Timestamp%20Format.rst
.. _`SECoP Issue 4: The Timeout SEC Node Property`: 004%20The%20Timeout%20SEC%20Node%20Property.rst
.. _`SECoP Issue 5: Definition of the term 'Qualifier'`: 005%20Definition%20of%20the%20term%20Qualifier.rst
.. _`SECoP Issue 6: Keep Alive`: 006%20Keep%20Alive.rst
.. _`SECoP Issue 7: Time Synchronization`: 007%20Time%20Synchronization.rst
.. _`SECoP Issue 8: Groups and Hierarchy`: 008%20Groups%20and%20Hierarchy.rst
.. _`SECoP Issue 9: Module Meaning`: 009%20Module%20Meaning.rst
.. _`SECoP Issue 10: Character set for Names`: 010%20Character%20set%20for%20Names.rst
.. _`SECoP Issue 11: Default parameters 'value' and 'target'`: 011%20Default%20parameters%20value%20and%20target.rst
.. _`SECoP Issue 12: JSON values vs documents`: 012%20JSON%20values%20vs%20documents.rst
.. _`SECoP Issue 13: Access Control`: 013%20Access%20Control.rst
.. _`SECoP Issue 14: Synchonisation of Hardware Modules on Different SEC Nodes`: 014%20Synchonisation%20of%20Hardware%20Modules%20on%20Different%20SEC%20Nodes.rst
.. _`SECoP Issue 15: Support for Start/Stop/Pause/Shutdown Commands`: 015%20Support%20for%20Start%20Stop%20Pause%20Shutdown%20Commands.rst
.. _`SECoP Issue 16: wanted_target Parameter`: 016%20wanted_target%20Parameter.rst
.. _`SECoP Issue 17: Collect Use Cases for 'starting'`: 017%20Collect%20Use%20Cases%20for%20starting.rst
.. _`SECoP Issue 18: Interface Classes`: 018%20Interface%20Classes.rst
.. _`SECoP Issue 19: Set SEC Node clock over SECoP`: 019%20Set%20SEC%20Node%20clock%20over%20SECoP.rst
.. _`SECoP Issue 20: PID tables`: 020%20PID%20tables.rst
.. _`SECoP Issue 21: Usage of JSON null`: 021%20Usage%20of%20JSON%20null.rst
.. _`SECoP Issue 22: Enable Module instead of Shutdown Command`: 022%20Enable%20Module%20instead%20of%20Shutdown%20Command.rst
.. _`SECoP Issue 23: Adjust Datatypes`: 023%20Adjust%20Datatypes.rst
.. _`SECoP Issue 24: Mix Commands within Parameters in the Description`: 024%20Mix%20Commands%20within%20Parameters%20in%20the%20Description.rst
.. _`SECoP Issue 25: Pong Format`: 025%20Pong%20Format.rst
.. _`SECoP Issue 26: More Module Meanings`: 026%20More%20Module%20Meanings.rst
.. _`SECoP Issue 27: Proposals for Interface Classes`: 027%20Proposals%20for%20Interface%20Classes.rst
.. _`SECoP Issue 28: Clarify buffering mechanism`: 028%20Clarify%20buffering%20mechanism.rst
.. _`SECoP Issue 29: New messages for buffering`: 029%20New%20messages%20for%20buffering.rst
.. _`SECoP Issue 30: Clarify Message parsing`: 030%20Clarify%20Message%20parsing.rst
.. _`SECoP Issue 31: Clarify timing requirements of status updates`: 031%20Clarify%20timing%20requirements%20of%20status%20updates.rst
.. _`SECoP Issue 32: Clarify specification using RFC-style definitions`: 032%20Clarify%20specification%20using%20RFC-style%20definitions.rst
.. _`SECoP Issue 33: Include section about security and access control`: 033%20Include%20section%20about%20security%20and%20access%20control.rst
.. _`SECoP Issue 34: Clarify 'do' message`: 034%20Clarify%20do%20message.rst
.. _`SECoP Issue 35: Partial Structs`: 035%20Partial%20Structs.rst
.. _`SECoP Issue 36: Dynamic units`: 036%20Dynamic%20units.rst
.. _`SECoP Issue 37: Clarification of status`: 037%20Clarification%20of%20status.rst
.. _`SECoP Issue 38: Extension mechanisms`: 038%20Extension%20mechanisms.rst
.. _`SECoP Issue 39: Use cases`: 039%20Use%20cases.rst
.. _`SECoP Issue 40: Finalise Specification`: 040%20Finalise%20Specification.rst
.. _`SECoP Issue 41: Handling of JSON-numbers`: 041%20Handling%20of%20JSON-numbers.rst
.. _`SECoP Issue 42: Requirements of datatypes`: 042%20Requirements%20of%20datatypes.rst
.. _`SECoP Issue 43: Parameters and units`: 043%20Parameters%20and%20units.rst
.. _`SECoP Issue 44: Scaled integers`: 044%20Scaled%20integers.rst
.. _`SECoP Issue 45: Async error updates`: 045%20Async%20error%20updates.rst
.. _`SECoP Issue 46: remote logging`: 046%20remote%20logging.rst
.. _`SECoP Issue 47: Error classes`: 047%20Error%20classes.rst
.. _`SECoP Issue 48: mode parameter`: 048%20mode%20parameter.rst
.. _`SECoP Issue 49: Precision of Floating Point Values`: 049%20Precision%20of%20Floating%20Point%20Values.rst
.. _`SECoP Issue 50: Reserved Names`: 050%20Reserved%20Names.rst
.. _`SECoP Issue 51: No Restrictions on Datatype Nesting`: 051%20No%20Restrictions%20on%20Datatype%20Nesting.rst
.. _`SECoP Issue 52: Include Some Properties into Datatype`: 052%20Include%20Some%20Properties%20into%20Datatype.rst
.. _`SECoP Issue 53: Custom Status Values`: 053%20Custom%20Status%20Values.rst
.. _`SECoP Issue 54: Splitting _update_ into two Message Types`: 054%20Splitting%20_update_%20into%20two%20Message%20Types.rst
.. _`SECoP Issue 55: Reformat Datatype description`: 055%20Reformat%20Datatype%20description.rst
.. _`SECoP Issue 56: Additional Busy States`: 056%20Additional%20Busy%20States.rst
.. _`SECoP Issue 58: use JSON key, value for describe message`: 058%20use%20JSON%20key%20value%20for%20describe%20message.rst
.. _`SECoP Issue 59: set_mode and mode instead of some commands`: 059%20set_mode%20and%20mode%20instead%20of%20some%20commands.rst
.. _`SECoP Issue 60: enhancements to the documentation`: 060%20enhancements%20to%20the%20documentation.rst
.. _`SECoP Issue 61: new predefined property implementation`: 061%20new%20predefined%20property%20implementation.rst
.. _`SECoP Issue 62: naming convention for related parameters`: 062%20naming%20convention%20for%20related%20parameters.rst
.. _`SECoP Issue 63: enumeration of floating point values`: 063%20enumeration%20of%20floating%20point%20values.rst
.. _`SECoP Issue 64: reading multiple parameters simultaneously`: 064%20reading%20multiple%20parameters%20simultaneously.rst
.. _`SECoP Issue 65: handling of coupled (sub)modules`: 065%20handling%20of%20coupled%20sub%20modules.rst
.. _`SECoP Issue 66: force re-connect`: 066%20force%20re-connect.rst
.. _`SECoP Issue 67: pid control parameters`: 067%20pid%20control%20parameters.rst
.. _`SECoP Issue 68: transaction of multiple commands`: 068%20transaction%20of%20multiple%20commands.rst
.. _`SECoP Issue 69: optional structs by default`: 069%20optional%20structs%20by%20default.rst
.. _`SECoP Issue 70: message is a JSON object`: 070%20message%20is%20a%20JSON%20object.rst
.. _`SECoP Issue 71: accessing sub items of parameters`: 071%20accessing%20sub%20items%20of%20parameters.rst
.. _`SECoP Issue 72: features`: 072%20features.rst
.. _`SECoP Issue 73: HasLimits and HasOffset`: 073%20HasLimits%20and%20HasOffset.rst
.. _`SECoP Issue 74: Standardization of units`: 074%20Standardization%20of%20units.rst
.. _`SECoP Issue 75: New messages check and checked`: 075%20New%20messages%20check%20and%20checked.rst
.. _`SECoP Issue 76: Interface for Measurable hardware`: 076%20Interface%20for%20Measurable%20hardware.rst
.. _`SECoP Issue 77: predefined parameter name prefixes`: 077%20predefined%20parameter%20name%20prefixes.rst
.. _`SECoP Issue 78: Interacting Modules - use case power supply`: 078%20Interacting%20Modules%20-%20use%20case%20power%20supply.rst
.. _`SECoP Issue 79: hints for readonly access`: 079%20hints%20for%20readonly%20access.rst
.. _`SECoP Issue 80: issues with software ramp`: 080%20issues%20with%20software%20ramp.rst
.. _`SECoP Issue 81: dynamic descriptive data`: 081%20dynamic%20descriptive%20data.rst
.. _`SECoP Issue 82: Feature HasRamp`: 082%20Feature%20HasRamp.rst
