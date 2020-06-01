#!/bin/sh

TAPDEV="$1"
BRIDGEDEV="bridge1"

ifconfig $BRIDGEDEV addm $TAPDEV
