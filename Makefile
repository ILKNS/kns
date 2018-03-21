CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	subdir-ccflags-y += -I$(PWD)/inc
	SRC  := $(subst $(PWD)/,,$(shell find $(PWD) -name "*.c"))
	OBJS  := $(subst .c,.o,$(SRC))
	ixgbe			:= ixgbe/ixgbe_api.o  ixgbe/ixgbe.o  ixgbe/ixgbe_common.o
	obj-y			:=  ixgbe/
	obj-m 			+=  kix.o
	kix-objs		:=  init.o $(OBJS)
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD  :=		$(shell pwd)

all: module
.PHONY: all

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
