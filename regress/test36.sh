#!/bin/sh

# test36: Create a tun1 persistent device and release it

TEST="`pwd`/helper36"
SYSTEM=`uname`
TARGET='tun1'
TYPE='tun'

if [ "$SYSTEM" = "Linux" ]; then
	IFDEL="ip tuntap del $TARGET mode $TYPE"
else
	IFDEL="ifconfig $TARGET destroy"
fi

OK=0
$TEST && OK=1

# The $TEST is successful
if [ $OK -eq 1 ]; then
	ifconfig $TARGET && OK=2
else
	return 1
fi

# The $TARGET still exists
if [ $OK -eq 2 ]; then
	$IFDEL
	return 0
else
	return 1
fi

