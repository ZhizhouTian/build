KERNEL_DIR=/home/zz/works/source
OUT=/media/zz/linux/works/vms/build/out

MODULE_NAME=test_ps_rss
obj-m += ${MODULE_NAME}.o

all:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` modules
clean:
	make -C ${KERNEL_DIR} O=${OUT} M=`pwd` clean
