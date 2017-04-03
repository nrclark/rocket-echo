MODULE_NAME := hello
SOURCES := hello-main.c dummy.c dummy.h

MODULE := $(MODULE_NAME).ko
MODULE_BUILD_DIR := /lib/modules/$(shell uname -r)/build

ifneq "$(filter $(MODULE_NAME).%,$(SOURCES))" ""
$(error Module name '$(MODULE_NAME)' is an invalid base filename for sources)
endif

OBJS := $(filter %.o,$(SOURCES:%.c=%.o))
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(OBJS)

ccflags-y := -O0 -g

module: $(MODULE)
all: module

$(MODULE): $(SOURCES)
	make -C $(MODULE_BUILD_DIR) M=$(PWD) modules

clean: unload
	make -C $(MODULE_BUILD_DIR) M=$(PWD) clean

# To pass load-time parameters to the module, call make as follows:
# make load params="param1 param2" param1=foo param2=bar
load: $(MODULE) unload
	sudo insmod $(MODULE) $(foreach x,$(params),$(if $($(x)),$(x)=$($(x)),))

info: $(MODULE)
	sudo modinfo $(MODULE)

unload:
	if lsmod | grep -qP "^$(MODULE_NAME)[ \t]*[0-9]+"; then \
		sudo rmmod $(MODULE); \
	fi

cycle:
	$(MAKE) load
	$(MAKE) unload
