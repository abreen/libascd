`libascd`
=========

`libascd` is the library that `asc` uses to communicate with a local
running ASC daemon (`ascd`). However, since it supplies very general
functionality for communicating with an ASC daemon, `libascd` can be
used by small utility programs, whose purpose may not at all involve
running programs speculatively. For example, the ASC shell (`ascsh`)
uses `libascd` to allow humans to access the database an ASC daemon
maintains, or to instruct an ASC daemon to shut down.

In a sense, `libascd` defines an application protocol that describes
how any `asc`-like client may communicate with an ASC daemon over its
UNIX control socket. It also describes how common ASC data structures
are serlialized. (As far as I understand it, this is similar in function
to [D-Bus](http://www.freedesktop.org/wiki/Software/dbus), for example.)

This library and the protocol it defines is used "locally" (i.e.,
on a single host). It is entirely different from the application protocol
that ASC daemons use to communicate with each other over the Internet.
(That protocol is known as the *ASC network protocol*.)


`libascd` messages
------------------

A `libascd` message can be one of the following types:

*   a *lookup* message (sent from a client to a daemon), which specifies
    some information about a program and represents a request by the
    client for learning data about that program,
*   a *store* message (sent from a client to a daemon), which allows a
    client to send learning data about a particular program to a daemon,
*   a *dump* message (sent from a client to a daemon) which instructs a
    daemon to prepare a complete "dump" of the database it maintains, or
*   a *quit* message (sent from a client to a daemon), which allows a
    client to instruct the daemon to shut down.

This information is sent as plain ASCII text over the UNIX control socket,
and is simple enough to be human-readable. A message consists of one or
more *parts*, each separated by a newline. The entire message ends with
a `NUL` byte. The first part must be one of the following: `lookup`,
`store`, `dump`, or `quit`, depending on the type of the message.
Then zero or more parts may follow.

Any binary data, if absolutely necessary to send, is encoded using Base64.
