Curly
=====

Curly is an experimental event-driven HTTP server, written in C.  I wrote it
because I realized that while I can read C very well, know what idiomatic C
looks like, have written a lot of C++, and I even interview people in C during
my day job, I had never actually written a nontrivial program in plain C.

Philosophy
----------

While this is intended to be efficient, secure, well-tested, production quality
code, I do not recommend that you use it for anything but educational purposes.
Everything Curly does, servers like [Nginx](http://nginx.org) probably do way
better.

Also, though the features of the C language are restricted to C99 only, the
code is probably not very portable. It uses several functions that are
available in Linux only.

Naming
------

The name was chosen by `aspell -l en dump master | grep ^c | shuf | less`. When
`curly` came up in the results, it seemed only too appropriate: it starts with
C, C is a curly-braces language, and it contains "URL" in the middle.

Building
--------

If you checked out from source, the following sequence of commands should build
Curly successfully and produce a binary in `src/curly`:

    ./bootstrap
    ./configure
    make

Use `./configure CC=colorgcc` if you have `colorgcc` installed and want
colourful output.

To run the tests:

    make test

License
-------

The code is under the three-clause BSD license; see the file `LICENSE` for
details.
