# pltermlib
`pltermlib` is a very simple text-mode/terminal UI library that is very
portable. It is meant to replace GNU Ncurses in embedded systems.

It uses ANSI escape sequences, which make it usable in even the most limited of
environments. It also makes it extremely portable to other platforms: all that
is needed to get `pltermlib` working on a new platform is just, assuming it has
a C99-compliant Standard C library implementation, a port of the `read` and
`write` POSIX syscalls to the target platform's counterparts, a way to change the
terminal's modes, and a terminal that supports ANSI escape codes.

# Build instructions

To build `pltermlib`, you must configure it first. To configure it, run the following:
```
./configure
```
After configuring it, you can finally build it by running:
```
./compile build
```

# Contributions

Currently, no contributions are being accepted. Contributions may open once release 1.00
comes out
