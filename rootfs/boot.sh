#!/bin/sh
export PATH=/bin
/bin/busybox --install /bin
mount -t proc "" /proc
mount -t devtmpfs "" /dev

exec init
