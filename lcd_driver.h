/* 
 * lcd_driver.h: holds all include statements, preprocessor constants
 *             data structures, global variables, and function prototypes
 *             used throughout this file.
 *
 * Brad Marquez
 * Aigerim Shintemirova
 * Joseph Rothlin
 *
 */
#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#define DATA_ 45
#define LATCH_ 47
#define CLOCK_ 67
#define RS_ 68
#define RW_ 44
#define E_GAME 60
#define E_SCORE 26

#define CHAR_PER_LINE 16
#define NUM_LINES 3

#define GAME_SCREEN 0
#define SCORE_SCREEN 1

static int EArr[2] = {E_GAME, E_SCORE};

#define DEVICE_NAME "lcd_driver"

/* data structures */
// contains data about the device.
// data : buffer for character data stored.
struct fake_device {
	char data[100];
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

void setBus(unsigned char);
void command(unsigned char, int);
void setBus(unsigned char);
void lcdSend(int);
void initialize(int);
void displayOff(int);
void clearDisplay(int);
void writeChar(unsigned char, int);
void setAddress(unsigned char, int);

/* operations usable by this file. */
static struct file_operations fops = {
   .owner = THIS_MODULE,
   .read = device_read,
   .write = device_write,
   .open = device_open,
   .release = device_close
};
#endif  // LCD_DRIVER_H_
