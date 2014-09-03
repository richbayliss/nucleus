nucleus - A simple UDP-based layer-2 tunnel daemon
==================================================

nucleus allows you to have multiple remote nodes joining a central layer 2 network over UDP. The purpose of nucleus is to create an easy to use, low overhead tunnel for use with WiFi hotspots - to get the traffic back to a central network over public infrastructure.

nucleus uses the Linux TUNTAP interface to create an endpoint which it can read and write - and then passes the data to/from a server instance. The single nucleus executable can be used in both client and server modes through the use of a config file.

Progress
========

nucleus is very much in the Alpha stage - it barely works. In time it will improve and become production ready.

Compile
=======

```
make
```

Run
===

```
nucleus {config file path}
```

Config File
===========

The config file consists of lines with the option name and value, seperated by whitespace, one per line.

There are few options at this time:
* **debug** 1 or 0 (default) - 1 shows debug output on the console, 0 prevents it.
* **listen** {int} - sets the port the UDP socket will listen for incoming packets on. Default is 0 (any port)
* **server** {ip} {port} - sets the program into client mode, sending all packets to the {ip}:{port} of the server
* **intf** {intf} - sets the name of the TAP interface to bind to. Default will create a new one for you, lasting the life of the app. Eg, tap0

