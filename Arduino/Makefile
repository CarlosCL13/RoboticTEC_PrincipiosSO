obj-m += arduino_driver.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

load:
	sudo insmod arduino_driver.ko

uninstall:
	sudo rmmod arduino_driver

kernel_msg:
	sudo dmesg | tail -n 20

module_loaded:
	lsmod | grep arduino_driver
all_modules:
	lsmod 

details_module:
	modinfo arduino_driver