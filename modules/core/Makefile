KERNEL_DIR=/home/zz/works/source
OUT=/media/zz/linux/works/vms/build/out
MODULE_DIR=/media/zz/linux/works/vms/build/modules/core

MODULE_NAME=core
obj-m += ${MODULE_NAME}.o
core-objs := src/main.o
core-objs += src/period_task.o

EXTRA_CFLAGS += -Wall -Werror -I${MODULE_DIR}/include

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
	cp core.ko /tftpboot -rf
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
	rm -rf /tftpboot/core.ko
