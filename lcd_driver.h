/* 
 * lcd_driver.h: holds all include statements, preprocessor constants
 *             data structures, global variables, and function prototypes
 *             used throughout this file.
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
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

// GPIO pins used for the shift register and LCDs
#define DATA_ 45
#define LATCH_ 47
#define CLOCK_ 67
#define RS_ 68
#define RW_ 44
#define E_GAME 60
#define E_SCORE 26

#define CHAR_PER_LINE 16
#define NUM_LINES 3

// screenSel values
#define GAME_SCREEN 0
#define SCORE_SCREEN 1

static int EArr[2] = {E_GAME, E_SCORE};

#define DEVICE_NAME "lcd_driver"

// contains data about the device.
// data : buffer for character data stored currently on the screen
struct device {
  char data[CHAR_PER_LINE * NUM_LINES];
  struct semaphore sem;
} virtual_device;

// stores info about this char device.
static struct cdev* mcdev;
// holds major and minor number granted by the kernel
static dev_t dev_num;

// Takes necessary steps to insert module into the OS, including printing
// necessary information for creating device file. Returns zero on success,
// non-zero on failure
static int __init driver_entry(void);

// Run when module is uninstalled, unregisters device
static void __exit driver_exit(void);

// Locks device, sets all GPIOs, and goes through initialization sequence for
// the LCD screens. Return negative on failure, 0 otherwise
static int  device_open(struct inode*, struct file*);

// Closes devices, clears displays, frees the GPIO pins, and returns access
// to semaphore.
static int device_close(struct inode*, struct file *);

// This function will take the contents of bufSource and print them to the
// LCD screens. The first 16 bytes are printed to the left screen,
// the next 16 bytes are printed to the top row of the right screen,
// and the last 16 bytes are printed to the bottom row of the right screen.
// Any other bytes will be ignored. providing a bufCount of 0 will clear screen
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

// Loads and sends data from the shift register
void setBus(unsigned char num);

// Loads data through the shift register and sends the command to the LCD
void command(unsigned char data, int screenSel);

void setBus(unsigned char num);

// Flips the enable switch on the LCD to execute the loaded instruction
// of the screen set by screenSel
void lcdSend(int screenSel);

// Initializes the LCD with the proper series of commands
void initialize(int screenSel);

// Turns the LCD selected by screenSel off
void displayOff(int screenSel);

// Clears the LCD selected by screenSel
void clearDisplay(int screenSel);

// Sets DB7 to DB0 to the given 8 bits of the screen set by screenSel
void writeChar(unsigned char character, int screenSel);

// Sets the R/W pointer to the address specified of the screenSel screen
void setAddress(unsigned char address, int screenSel);

// operations usable by this file
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .write = device_write,
  .open = device_open,
  .release = device_close
};

#endif  // LCD_DRIVER_H_
