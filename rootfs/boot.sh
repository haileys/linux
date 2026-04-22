#!/bin/sh
export PATH=/bin
/bin/busybox --install /bin
mount -t proc "" /proc

exec sh
