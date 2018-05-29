Introduction
------------

This is a daemon, which provides OSC to DMX connectivity.
It transmits a buffer of DXM values to DMX interface about 30 times per second.
OSC messages can be sent to port 7777 to change te contents of the buffer.

It uses libftdi1 to talk to the DMX interface and liblo to implement OSC.
