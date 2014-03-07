Curly
=====

Curly is an experimental HTTP server, written in pure C. I wrote it because I
realized that while I can read C very well, know what idiomatic C looks like,
have written a lot of C++, and I even interview people in C during my day job,
I had actually never written a nontrivial program in plain C.

While this is intended to be production quality code, I do not recommend that
you use it for anything but educational purposes. Everything Curly does,
servers like [Nginx](http://nginx.org) probably do way better.

The name was chosen by `aspell -l en dump master | grep ^c | shuf | less`. When
`curly` came up in the results, it seemed only too appropriate.
