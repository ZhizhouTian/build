WORKDIR=/home/zz/works/source
KERNEL_DIR=${WORKDIR}/linux
OUT=${WORKDIR}/build/out

MODULE_NAME=ikbr
obj-m += ${MODULE_NAME}.o
#ikbr-objs := ikbr_host.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
