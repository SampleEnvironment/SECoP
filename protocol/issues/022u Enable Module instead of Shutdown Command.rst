SECoP Issue 22: Enable Module instead of Shutdown Command
=========================================================

Ideas
-----

See extract of the following E-mail discussion:

Markus wrote::

    I take this for discussing about the shutdown command again. Would it not
    be more sensible, instead of having a shutdown command on a module, to implement
    it as an 'enable' module, which is a boolean drivable?

    I have taken this idea from the 'amagnet' from MLZ.

    For the following I use the term 'device' for a group of modules, which are attached to
    the same hardware.

    - enable:value is false when the SEC-node is not yet initialized
    - setting enable:target to true will initialize.
    - the device is is initialized, when enable:status is idle and enable:value is true.
    - setting enable:target to false will shut down.
    - the device is in 'shut down' state when enable:value is false and enable:status is idle

    There might be devices which initialize automatically on power on, on these
    setting enable to true is only needed if you change your mind after shutting
    the device down.

    We need less status values and commands with such an initialization / shutdown mechanism.
    I think it is a good philosophy to put anything for which we can wait into a
    separate module. However, as Enno proposed in a private discussion, we might
    define a status value "disabled" (might be a substate of 'error'). This would
    be the state of all modules of the device, when it is shut down.

    The standard might just describe this as an interface class called 'Enable'.
    In principle there might several modules with this class on a SEC-Node.

    If we introduce 'shutdown' as a SEC-Node command, it would not allow to have
    several groups of modules with each of them having a separate 'shutdown' command.

Enrico wrote::

    interesting idea, Markus!
    Initially I thought this would be a good thing to put as parameter of the secnode.
    But if you want to have several enables for different sets/groups of modules this won't
    work directly.

    I think we should keep this as an topic, but not define anything so far, as I feel this
    would overcomplicate the current state. Once everyone got used to 'the secop way', it is
    worth thinking about it. Could also be a convention like:

    secnode-parameter name = 'enable_' + name of group

    and this then enables/disables all modules from that specific group.

    Just my 0.02€.
