#!/bin/sh

# test33: Create a tap0 persistent device and destroy it

TEST="$(pwd)/helper33"
TARGET='tap0'
TYPE='tap'

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

# The interface should NOT exists
if ifcheck "$TARGET"; then
	ifdel "$TARGET" "$TYPE"
	exit 1
fi
exit 0
