KERNEL_DIR=/home/zz/works/source
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=us
obj-m += ${MODULE_NAME}.o
us-objs := main.o
us-objs := url_secure.o
us-objs += protobuf-c.o
us-objs += url_secure_whitelist.o
us-objs += urlwhitelist.pb-c.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
