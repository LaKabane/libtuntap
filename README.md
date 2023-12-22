# libtuntap 0.4

libtuntap is a library for configuring TUN or TAP devices in a portable manner.

## Contents

1. [Description](#description)
2. [Install](#install)
3. [Notes](#notes)
4. [Contributing](#contributing)
5. [License](#license)

## Description

TUN and TAP are virtual networking devices which allow userland applications
to receive packets sent to it. The userland applications can also send their
own packets to the devices and they will be forwarded to the kernel.

This is useful for developping tunnels, private networks or virtualisation
systems.

#### Supported Features

   * Creation of TUN _and_ TAP devices;
   * Autodetection of available TUN or TAP devices;
   * Setting and getting the MAC address of the device;
   * Setting and getting the MTU of the device;
   * Setting the status of the device (up/down);
   * Setting the IPv4 address and netmask of the device;
   * Setting the persistence mode of the device;
   * Setting the name of the device (Linux only);
   * Setting the description of the device (OpenBSD and FreeBSD only);
   * Wrapper libraries for other languages.

#### Supported Systems

   * OpenBSD;
   * Linux;
   * NetBSD;
   * Darwin (up to High Sierra).

#### Current Porting Efforts

   * Windows;
   * FreeBSD.

#### In the future

   * AIX;
   * Solaris.

## Install

#### Requires

* cmake;
* C and C++ compilers.

#### Build

This project is built with cmake:

    $ mkdir build; cd build
    $ cmake ../
    $ make
    # make install

It is possible to tweak the destination folder for the install rule with the
environment variable `DESTDIR`.  The default behaviour is to install under
the `/usr/lib` folder for Linux and `/usr/local/lib` for everyone else.

Example make invocation:

    $ DESTDIR=/tmp make install

The following options can be tweaked:

- `ENABLE_CXX`: Enable building of the C++ wrapper library libtuntap++;
- `ENABLE_PYTHON`: Enable building of the Python wrapper library pytuntap;
- `BUILD_TESTING`: Enable building of the regress tests;
- `BUILD_SHARED_LIBS`: Build shared libraries instead of static ones.

If you want to build it for a release, additionally use `-DCMAKE_BUILD_TYPE=Release` as cmake argument.

#### Other languages bindings

We currently provide wrappers for two other languages: C++ and Python,
respectively named libtuntap++ and pytuntap.  More instructions about them is
provided in the [`bindings`](bindings/README.md) folder.

The C++ library is built by default and can be disabled with the flag
`ENABLE_CXX`.

The Python library is disabled by default and requires both `ENABLE_CXX` and
`ENABLE_PYTHON` to work.  You will also need Python 3.6 and Boost libraries.

Example cmake invocation:

    $ mkdir build; cd build
    $ cmake -D ENABLE_CXX=ON -D ENABLE_PYTHON=ON ../

#### Tests

A series of regress tests can be built with the `BUILD_TESTING` option.
They are enabled by default.  A list and a description for each of them can
be found in the [`regress`](regress/README.md) folder.

Example cmake invocation:

    $ mkdir build; cd build
    $ cmake -D BUILD_TESTING=ON ../
    # make test

#### Static or shared

Up to version 0.3 the libtuntap `CMakeFiles.txt` exported two libraries: one shared, one static. To simplify the building of the wrapper libraries it was decided to only build one. The default is to build a static library but this behaviour can be changed with the option `BUILD_SHARED_LIBS`.

Example cmake invocation:

    $ mkdir build; cd build
    $ cmake -D BUILD_SHARED_LIBS=ON ../
    $ make

#### Local configurations

The main `CMakeFiles.txt` includes an optional `CMakeFiles.txt.local` which can be used to store persistent options across builds.

Example:

    $ cat CMakeLists.txt.local
    set(BUILD_SHARED_LIBS ON)
    set(BUILD_TESTING OFF)
    set(ENABLE_CXX OFF)

## Notes

#### Notes for Mac OS X users

You need to install the tuntaposx project for this library to be useful,
which is a third-party kext.

### Notes for Windows users

You need to install the tap-windows driver provided by the [OpenVPN project](https://openvpn.net/index.php/open-source/downloads.html).

## Contributing

Feel free to open issues and pull-requests, even if we are bad at replying
on time.  You can also chat with us on the XMPP network: libtuntap channel
at rooms.bouledef.eu.

## License

All the code is licensed under the ISC License.
It's free, not GPLed !
