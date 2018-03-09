KERNEL_DIR=/media/zz/linux/works/vms/kernel-stable/
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=url_secure
obj-m += ${MODULE_NAME}.o
url_secure-objs := main.o
url_secure-objs += protobuf-c.o
url_secure-objs += url_secure_whitelist.o
url_secure-objs += urlwhitelist.pb-c.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
