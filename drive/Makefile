obj-m += virtual_mouse.o
    CURRENT_PATH:=$(shell pwd)
    LINUX_KERNEL_PATH:=/lib/modules/$(shell uname -r)/build
all:

	@echo $(MAKE)
	
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
	
	@echo $(MAKE)
	
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions
