#!/bin/bash

WORKDIR=$(pwd)
BUSYBOX_DIR=${WORKDIR}/../busybox
IPTABLES_DIR=${WORKDIR}/../iptables

#do clean works
rm -rf tmpfs
rm -rf ramdisk*
rm -rf _rootfs
mkdir _rootfs

# copy
cp -rf rootfs/* _rootfs/
cp $BUSYBOX_DIR/_install/*  _rootfs/ -raf
cp $IPTABLES_DIR/out/* _rootfs/ -raf

mkdir -p _rootfs/etc/init.d
mkdir -p _rootfs/proc/
mkdir -p _rootfs/modules/
mkdir -p _rootfs/sys/
mkdir -p _rootfs/tmp/
mkdir -p _rootfs/root/
mkdir -p _rootfs/var/
mkdir -p _rootfs/mnt/
mkdir -p _rootfs/lib64
mkdir -p _rootfs/dev/
mkdir -p _rootfs/run/

if [ -d ${WORKDIR}/modules ]; then
	find ${WORKDIR}/modules/ -name "*.ko" -exec cp {} _rootfs/modules/ \;
fi
cp -arf /lib/x86_64-linux-gnu/* _rootfs/lib64/
rm -rf _rootfs/lib/*.a

sudo mknod _rootfs/dev/tty1 c 4 1
sudo mknod _rootfs/dev/tty2 c 4 2
sudo mknod _rootfs/dev/tty3 c 4 3
sudo mknod _rootfs/dev/tty4 c 4 4
sudo mknod _rootfs/dev/console c 5 1
sudo mknod _rootfs/dev/null c 1 3

sudo dd if=/dev/zero of=ramdisk bs=1M count=32
sudo mkfs.ext4 -F ramdisk
sudo mkdir -p tmpfs
sudo mount -t ext4 ramdisk ./tmpfs/  -o loop
sudo cp -raf _rootfs/*  tmpfs/
sudo umount tmpfs
sudo gzip --best -c ramdisk > ramdisk.gz

rm -rf _rootfs
rm -rf tmpfs
rm -rf ramdisk
