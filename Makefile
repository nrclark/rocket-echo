obj-m += hello-1.o

MOD_MAKE := make -C /lib/modules/$(shell uname -r)/build M=$(PWD)

all:
	$(MOD_MAKE) modules

clean:
	$(MOD_MAKE) clean
