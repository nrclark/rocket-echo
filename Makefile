MODULE_NAME := hello
SOURCES := hello.c dummy.c

MODULE_BUILD_DIR := /lib/modules/$(shell uname -r)/build
EXTRA_OBJS := $(filter-out $(MODULE_NAME).%,$(SOURCES:%.c=%.o))
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(EXTRA_OBJS)

all:
	make -C $(MODULE_BUILD_DIR) M=$(PWD) modules

clean:
	make -C $(MODULE_BUILD_DIR) M=$(PWD) clean

load:
	sudo insmod $(MODULE_NAME).ko

unload:
	sudo rmmod $(MODULE_NAME).ko
