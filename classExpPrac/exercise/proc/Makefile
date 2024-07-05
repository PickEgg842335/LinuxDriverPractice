obj-m += procfs_read.o
obj-m += procfs_mkdir.o
obj-m += procfs_write.o
obj-m += procfs_fops.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
