obj-m += time_unit.o

all:
	make M=`pwd` -C /lib/modules/`uname -r`/build modules
clean:
	make M=`pwd` -C /lib/modules/`uname -r`/build clean
