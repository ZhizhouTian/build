KERNEL_DIR=/media/zz/linux/works/vms/kernel-stable/
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=test_md5
obj-m += ${MODULE_NAME}.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
