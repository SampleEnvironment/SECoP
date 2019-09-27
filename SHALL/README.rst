SHALL - SECoP hardware abstraction library layer
================================================

This libraries aim a convenient way to the world of `SECoP`_.
With existing programs and drivers inside the sample environment, you do
not want to rewrite all of your code, you probably want to extend it with
SECoP. Then *SHALL* comes in and allows different ways to export or
import data without much knowledge of SECoP internals.

See also <https://github.com/SampleEnvironment/SECoP>.

The SEC node (server side) provides modules (e.g. a temperature controller)
with some accessibles, which could be some parameters (e.g. actual
temperature, target temperature, status) and commands (e.g. go, stop).
There is a test program, which uses the library for creating a full SEC
node enriched with meta data and there are only a few points, where you
have to fill or fetch in your data.

There is also an idea of a ECS library (client side), but this is currently
under development - the client library is useful for the test program only.
There will be some effort to fix the library interface and to implement a
easy-to-use client.

Both (server and client library) make use of SECoP data, which is handled
by the variant library. It allows to create, parse, export, import, read or
modify SECoP data and it allocates and frees memory for you.

Because of compatibility to other languages, the libraries also have
*C*-compatible interfaces, which could be called easily. There is some
*LabVIEW* code, which makes use of it.

*SHALL* is written in *C++* using

* the Qt library <https://www.qt.io/> and
* the JSON library from Niels Lohmann <https://nlohmann.me>.

For distribution of *SHALL*, you will need the Qt libraries only.

.. _SECoP: https://github.com/SampleEnvironment/SECoP
