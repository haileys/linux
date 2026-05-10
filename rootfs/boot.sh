#!/bin/sh
echo "boot.sh start"
export PATH=/bin
echo "boot.sh busybox install"
/bin/busybox --install /bin
echo "boot.sh mounts"
mount -t proc "" /proc
mount -t devtmpfs "" /dev
echo "boot.sh exec init"
# mknod /rootfs b 254 0
# mount /rootfs /mnt
exec init
