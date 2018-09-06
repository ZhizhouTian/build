#!/bin/bash

WORKDIR=/home/zz/works/source

$WORKDIR/qemu/build/x86_64-softmmu/qemu-system-x86_64 \
	     -S \
	     -name QEMUGuest0 \
	     -enable-kvm \
	     -smp 4 \
	     -gdb tcp::1234 \
	     -serial telnet::1235,server,nowait \
	     -monitor telnet::1236,server,nowait \
	     -m 1024M \
	     -netdev tap,id=hn0,ifname=tap0,script=no,downscript=no \
	     -device e1000,id=e0,netdev=hn0,mac=52:a4:00:12:78:66 \
	     -kernel ./out/arch/x86_64/boot/bzImage \
	     -nographic \
	     -initrd ./ramdisk.gz \
	     -append "root=/dev/ram0 rw rootfstype=ext4 console=ttyS0 init=/linuxrc skversion=3"


#	     -netdev bridge,id=tap0,br=br0 \
#	     -netdev tap,id=hn0,vhost=off,script=./qemu-ifup,downscript=no \
#	     -netdev user,id=hn0,host=192.168.3.101,net=192.168.3.0/24,dhcpstart=192.168.3.102 \
#	     -device e1000,netdev=user.0 -netdev user,id=user.0,hostfwd=tcp::5555-:22 \
