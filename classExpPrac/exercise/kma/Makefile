obj-m += kma.o

#EXTRA_CFLAGS +=-DDEPMOD

all:
	make M=`pwd` -C /lib/modules/`uname -r`/build modules

clean:
	make M=`pwd` -C /lib/modules/`uname -r`/build clean
