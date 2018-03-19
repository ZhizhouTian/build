KERNEL_DIR=/media/zz/linux/works/vms/kernel-stable/
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=actest
obj-m += ${MODULE_NAME}.o
actest-objs := ahocorasick.o
actest-objs += mpool.o
actest-objs += node.o
actest-objs += replace.o
actest-objs += test_actrie.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
