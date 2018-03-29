CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	subdir-ccflags-y += -I$(PWD)/inc
	SRC  := $(subst $(PWD)/,,$(shell find $(PWD) -name "*.c"))
	OBJS-1  := $(subst .c,.o,$(SRC))
	OBJS  := $(filter-out user/hugepage_user.o, $(OBJS-1))
	obj-y			:=  ixgbe/ core/
	obj-m 			+=  kns.o
	kns-objs		:=  init.o $(OBJS)
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD  :=		$(shell pwd)
.PHONY: all
all: module hgpage

module:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

hgpage: user/hugepage_user.c
	cc user/hugepage_user.c -o user/hugepage_user.o -I$(PWD)/inc

endif

.PHONY: clean
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
