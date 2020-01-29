#!/bin/sh

# test36: Create a tun1 persistent device and release it

TEST="$(pwd)/helper36"
TARGET='tun1'
TYPE='tun'

if [ "$(uname)" = Linux ]; then
	ifdel() { ip tuntap del "$1" mode "$2"; }
	ifcheck() { ip link show "$1"; }
else
	ifdel() { ifconfig "$1" destroy; }
	ifcheck() { ifconfig "$1"; }
fi

if ! $TEST; then
	exit 1
fi

# The interface SHOULD exists
if ! ifcheck "$TARGET"; then
	exit 1
fi
ifdel "$TARGET" "$TYPE"
exit 0
