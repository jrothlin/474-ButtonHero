/* 
 * button_driver.h: holds all include statements, preprocessor constants
 *             data structures, global variables, and function prototypes
 *             used throughout this file.
 *
 * Brad Marquez
 * Aigerim Shintemirova
 * Joseph Rothlin
 *
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define UP 65
#define DOWN 48
#define LEFT 20
#define RIGHT 46
#define PRESS 49

#define DEVICE_NAME "button_driver"

#define NUM_BUTTONS 5

/* data structures */
// contains data about the device.
// data : buffer for character data stored.
struct fake_device {
	int status[NUM_BUTTONS];
	struct semaphore sem;
} virtual_device;

/* global variables */
// stores info about this char device.
static struct cdev* mcdev;
// holds major and minor number granted by the kernel
static dev_t dev_num;

/* function prototypes */
// file operations
static int __init driver_entry(void);
static void __exit driver_exit(void);
static int  device_open(struct inode*, struct file*);
static int device_close(struct inode*, struct file *);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

/* operations usable by this file. */
static struct file_operations fops = {
   .owner = THIS_MODULE,
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_close
};
#endif  // BUTTON_DRIVER_H_
