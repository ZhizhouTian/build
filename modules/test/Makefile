WORKDIR=/home/zz/works/source
KERNEL_DIR=${WORKDIR}/linux
OUT=${WORKDIR}/build/out

MODULE_NAME=test_enc_gwid
obj-m += ${MODULE_NAME}.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
