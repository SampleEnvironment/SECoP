Motivation
==========

The main goal of the "committee for the standardization of sample environment
communication" is to establish a common standard protocol **SECoP** for
interfacing sample environment equipment to experiment control software.

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

.. rubric:: Other requirements

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
