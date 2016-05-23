export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
ccflags-y := -std=gnu99

obj-m += lcd_driver.o
obj-m += button_driver.o

# Kernel source directory
KDIR = /home/WorkStation6/Documents/EE474Lab12/Lab3/bb-kernel/KERNEL
PWD = $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
buttonHero: buttonHero.c buttonHero.h
	arm-linux-gnueabihf-gcc -o buttonHero buttonHero.c
#clean:
#	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
