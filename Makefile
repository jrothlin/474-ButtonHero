export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
ccflags-y := -std=gnu99

obj-m += lcd_driver.o
obj-m += button_driver.o

# Kernel source directory
KDIR = /home/WorkStation6/Documents/EE474Lab12/Lab3/bb-kernel/KERNEL
PWD = $(shell pwd)

all: default ButtonHero

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

ButtonHero: ButtonHero.o
	arm-linux-gnueabihf-gcc -Wall -o ButtonHero ButtonHero.o

ButtonHero.o: ButtonHero.c ButtonHero.h
	arm-linux-gnueabihf-gcc -Wall -c ButtonHero.c

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
