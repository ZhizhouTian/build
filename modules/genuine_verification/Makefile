KERNEL_DIR=/media/zz/linux/works/vms/kernel-stable/
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=gv
obj-m += ${MODULE_NAME}.o
${MODULE_NAME}-objs := gv_core.o ma01.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
