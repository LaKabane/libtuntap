#!/bin/sh

# test33: Create a tap0 persistent device and destroy it

TEST="`pwd`/helper33"
SYSTEM=`uname`

# There is no tap driver on OpenBSD
if [ "$SYSTEM" = "OpenBSD" ]; then
	TARGET='tun0'
	TYPE='tap'
else
	TARGET='tap0'
	TYPE='tap'
fi

if [ "$SYSTEM" = "Linux" ]; then
	IFDEL="ip tuntap del $TARGET mode $TYPE"
else
	IFDEL="ifconfig $TARGET destroy"
fi

OK=0
$TEST && OK=1

# If the $TEST was a success, check if the interface still exist
if [ $OK -eq 1 ]; then
	ifconfig $TARGET && OK=2
else
	return 1
fi

# Everything went fine
if [ $OK -eq 2 ]; then
	$IFDEL
	return 0
fi

# The $TARGET still exists, return failure
return 1
