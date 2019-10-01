SHALL - SECoP hardware abstraction layer library
================================================

The aim of these libraries is to provide a convenient entry into the world of `SECoP`_.
With existing programs and drivers inside the sample environment, you do
not want to rewrite all of your code, you probably want to extend it with
SECoP. This is where *SHALL* comes in, it facilitates different ways to export or
import data without much knowledge of SECoP internals.

See also <https://github.com/SampleEnvironment/SECoP>.

The SEC node (server side) provides modules (e.g. a temperature controller)
with some accessibles, which could be parameters (e.g. actual
temperature, target temperature, status) and commands (e.g. go, stop).
There is a test program which uses the library to create a full SEC
node enriched with meta data, with only a few points where you
have to fill or fetch in your data.

There is also an idea for an ECS library (client side), but this is currently
under development - the client library is useful for the test program only.
There will be some effort to fix the library interface and to implement an
easy-to-use client.

Both (server and client libraries) make use of SECoP data, which is handled
by the variant library. It allows the user to create, parse, export, import, read or
modify SECoP data and handles memory allocation and freeing.

Because of compatibility with other languages, the libraries also have
*C*-compatible interfaces, which can be called easily. There is some
*LabVIEW* code, which makes use of these.

*SHALL* is written in *C++* using

* the Qt library <https://www.qt.io/> and
* the JSON library from Niels Lohmann <https://nlohmann.me>.

For distribution of *SHALL*, you will need the Qt libraries only.

.. _SECoP: https://github.com/SampleEnvironment/SECoP
