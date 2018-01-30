SECoP Issue 3: SECoP Timestamp Format (closed)
==============================================

Proposals for the timestamp format are:

ISO time format
---------------

A date+time string in ISO format like "2017-06-21T13:30:01.123456+02:00"

The fractional seconds are optional, but the timezone has to be given. Z is allowed instead of +00:00.

Advantages:

  * human readable

Disadvantages:

  * needs more conversion efforts, as the time is internally already stored as numbers on mosts systems (supporting math operations).
  * although the ISO standard is clearly defined, there is a risk that time zones and daylight saving time is not handled properly

Fractional Unix Time
--------------------

Seconds since 1970-01-01T00:00:00+00:00, represented as a number. When converted to a IEEE double, a resolution of 1 usec can be kept for dates up to 2112.

Advantages:

  * if a double is used as an internal representation, no conversion is needed. using a double as an internal time representation has the advantage, that math operations can be done for free.
  * relative times for systems with no synchronized clock can be represented easily

Disadvantages:

  * not human readable (or at least not easily: time differences in seconds are still visible)


Discussion
----------

1) Human readibility was judged less important than easy implementaion by the majority.

2) Implementing relative times is also easier.

Decision
--------

At the meeting in Berlin (2017-05-30) the attendes decided for "Fractional Unix Time".
