# Makefile for Rocket-Echo sample kernel module.
# Written by: Nicholas Clark
# Written on: 03-April-2017

MODULE_NAME := rocket
SOURCES := src/rocket-echo.c src/rkt_buf.c src/_rkt_buf.h

MODULE := $(MODULE_NAME).ko
MODULE_BUILD_DIR := /lib/modules/$(shell uname -r)/build

ifneq "$(filter $(MODULE_NAME).%,$(SOURCES))" ""
$(error Module name '$(MODULE_NAME)' is an invalid base filename for sources)
endif

OBJS := $(filter %.o,$(SOURCES:%.c=%.o))
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := $(OBJS)

ccflags-y := -O0 -g -Wall

define \n


endef

.PHONY: module all
module: $(MODULE)
all: module

$(MODULE): $(SOURCES)
	make -C $(MODULE_BUILD_DIR) M=$(PWD) modules

.PHONY: clean
clean: unload
	make -C $(MODULE_BUILD_DIR) M=$(PWD) clean
	rm -rf doc

# To pass load-time parameters to the module, call make as follows:
# make load params="param1 param2" param1=foo param2=bar
.PHONY: load
load: $(MODULE) unload
	sudo insmod $(MODULE) $(foreach x,$(params),$(if $($(x)),$(x)=$($(x)),))

.PHONY: info
info: $(MODULE)
	sudo modinfo $(MODULE)

.PHONY: unload
unload:
	if lsmod | grep -qP "^$(MODULE_NAME)[ \t]*[0-9]+"; then \
		sudo rmmod $(MODULE); \
	fi

.PHONY: cycle
cycle:
	$(MAKE) load
	$(MAKE) unload

.PHONY: format
format: $(SOURCES)
	$(foreach x,$^,sed -ri 's/[ \t]*$$//g' $x$(\n))
	$(foreach x,$^,astyle $x --style=linux --suffix=none$(\n))

.PHONY: doc
doc: doc/html/index.html

doc/html/index.html: Doxyfile $(SOURCES)
	doxygen $<
