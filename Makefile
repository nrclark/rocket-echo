MODULE_NAME := hello
SOURCES := hello-main.c dummy.c

MODULE := $(MODULE_NAME).ko
MODULE_BUILD_DIR := /lib/modules/$(shell uname -r)/build

ifneq "$(filter $(MODULE_NAME).%,$(SOURCES))" ""
$(error Module name '$(MODULE_NAME)' is an invalid base filename for sources)
endif

OBJS := $(SOURCES:%.c=%.o)
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(OBJS)

ccflags-y := -O0 -g

module: $(MODULE)
all: module

$(MODULE):
	make -C $(MODULE_BUILD_DIR) M=$(PWD) modules

clean: unload
	make -C $(MODULE_BUILD_DIR) M=$(PWD) clean

load: $(MODULE) unload
	sudo insmod $(MODULE) $(loadparams)

info: $(MODULE)
	sudo modinfo $(MODULE)

unload:
	if lsmod | grep -qP "^$(MODULE_NAME)[ \t]*[0-9]+"; then \
		sudo rmmod $(MODULE); \
	fi

cycle:
	$(MAKE) load
	$(MAKE) unload
