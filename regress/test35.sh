#!/bin/sh

# test35: Create a tap0 persistent device and release it

TEST="$(pwd)/helper35"
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

# The interface SHOULD exists
if ! ifcheck "$TARGET"; then
	exit 1
fi
ifdel "$TARGET" "$TYPE"
exit 0
