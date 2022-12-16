# pltermlib
`pltermlib` is a very simple text-mode/terminal UI library that is very
portable. It is meant to replace GNU ncurses in embedded systems

It uses ANSI escape sequences, which make it usable in even the most limited of
environments. It also makes it extremely portable to other platforms: all that
is needed to get `pltermlib` working on a new platform is just, assuming it has
a C99-compliant Standard C library implementation, a port of the `read` and
`write` POSIX syscalls to the target platform's counterparts, and a terminal
that supports ANSI escape codes

`term.h`, the current implementation of `pltermlib` is currently being 
restructured to resemble a proper library tree
