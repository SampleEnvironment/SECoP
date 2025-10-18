Introduction
============

The main goal of the "committee for the standardization of sample environment
communication" is to establish a common standard protocol **SECoP** for
interfacing sample environment equipment to experiment control software.

Definition: Experiment Control Software (ECS)
    Software controlling the hardware for carrying out an experiment. Includes
    the user interface.  Usually speaks several protocols with different parts
    of the instrument.  Often also called "instrument control" in short.

There is a task (7.1) within the European framework SINE2020 also dealing with
this subject. In its description we read:

    … The standard should be defined in a way that it is compatible with a broad
    variety of soft- and hardware operated at the different large scale
    facilities. … The adoption of this standard will greatly facilitate the
    installation of new equipment and the share of equipment between the
    facilities. …

This also covers the aims of the committee.

The idea is that a sample environment apparatus can easily be moved between
facilities and instruments/beamlines.  As long as the facilities have
implemented a SECoP client within its ECS, and on the apparatus a SECoP server
is implemented, using the apparatus for an experiment should be straightforward.
An ECS can be built in such a way that the configuration of the apparatus may be
as short as entering a network address, as the description can be loaded over
the protocol.

Definition: Sample Environment Control Node (SEC node)
    Computing unit or process, connected to all control units (temperature
    controller, flow controller, pressure sensor ...) of a sample environment,
    bridge to the ECS.  SECoP specifies how ECS speak with a SEC node.  The SEC
    node allows ECS to access a set of modules (and their parameters/commands)
    via SECoP.  It also provides a list of accessible modules and parameters as
    well as descriptive meta data.


Other requirements
------------------

- The protocol should be easy to use.

- It should be easy to implement in connection with existing ECSs and sample
  environment control software.

- It should be possible to be implemented on the most common platforms
  (operating systems and programming languages).

- The protocol should be defined in way that allows a maximum **compatibility**:
  Newer and older versions of the syntax should be compatible.

- The protocol should be defined in a way that allows maximum **flexibility**: A
  simple (= equipped with minimal functionality) ECS implementation should be
  able to communicate with a complex SEC node (with wide-ranging functionality),
  and an ECS with extensive functionality should be able to deal with a simple
  SEC node that implements only a minimum of features.
