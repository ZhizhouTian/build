WORKDIR=/home/zz/works/source
KERNEL_DIR=${WORKDIR}/linux
OUT=${WORKDIR}/build/out

MODULE_NAME=ik_http
obj-m += ${MODULE_NAME}.o
ik_http-objs := http_parser.o
ik_http-objs += http.o
ik_http-objs += http_hash.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
