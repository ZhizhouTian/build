KERNEL_DIR=/media/zz/linux/works/vms/kernel-stable/
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=web_first
obj-m += ${MODULE_NAME}.o
web_first-objs += ahocorasick.o
web_first-objs += protobuf-c.o
web_first-objs += urllist.pb-c.o
web_first-objs += high_prio_url.o
web_first-objs += netlink_kernel.o
web_first-objs += ik_utils.o
web_first-objs += mpool.o
web_first-objs += node.o
web_first-objs += replace.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
	gcc netlink_app.c -o netlink_app
	cp netlink_app ../../rootfs/
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
	rm -rf netlink_app
