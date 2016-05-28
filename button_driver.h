/* 
 * button_driver.h
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
 *
 * Holds all the functions definitions, included files, and variable
 * definitions for button_driver.h
 */

#ifndef _BUTTON_DRIVER_H_
#define _BUTTON_DRIVER_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/gpio.h>
#include <linux/delay.h>

// GPIO inputs from button
#define UP 65
#define DOWN 48
#define LEFT 20
#define RIGHT 46
#define PRESS 49

#define DEVICE_NAME "button_driver"

#define NUM_BUTTONS 5

// Contains data about the device.
struct device {
  int status[NUM_BUTTONS];
  struct semaphore sem;
} virtual_device;

// Stores info about this char device.
static struct cdev* mcdev;
// Holds major and minor number granted by the kernel
static dev_t dev_num;

// Takes necessary steps to insert module into the OS, including printing
// necessary information for creating device file. Returns zero on success,
// non-zero on failure
static int __init driver_entry(void);

// Run when module is uninstalled, unregisters the device.
static void __exit driver_exit(void);

// Requests and sets up necessary GPIOs, returns negative on error, 0 otherwise
static int  device_open(struct inode*, struct file*);

// Closes device, frees the GPIO pins, and returns access to semaphore.
static int device_close(struct inode*, struct file *);

// Called when user wants to get state of the button input
// Warning: calling read from this module without specifying the correct
// number of bytes will result in no data being transferred
static ssize_t device_read(struct file*, char*, size_t, loff_t*);

// Operations usable by this file.
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = device_read,
  .open = device_open,
  .release = device_close
};

#endif  // _BUTTON_DRIVER_H_
