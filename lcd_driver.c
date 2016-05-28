/*
 * lcd_driver.c
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
 * 
 * This files is a Linux kernal driver for a circuit consisting of two LCD
 * screens and a serial in/parallel out shift register.
 * Target device is Beaglebone Black microcontroller.
 */

#include "lcd_driver.h"

/********************* FILE OPERATION FUNCTIONS ***************/

// Takes necessary steps to insert module into the OS, including printing
// necessary information for creating device file. Returns zero on success,
// non-zero on failure
static int __init driver_entry(void) {
  // Registering the device with the system
  int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
  if (ret < 0) {
    printk(KERN_ALERT "lcd_driver: Failed to allocate a major number\n");
    return ret;
  }
  printk(KERN_INFO "lcd_driver: major number is %d\n", MAJOR(dev_num));
  printk(KERN_INFO "Use \"mknod /dev/%s c %d 0\" for device file\n", DEVICE_NAME, MAJOR(dev_num));

  // Create cdev structure
  mcdev = cdev_alloc();
  mcdev->ops = &fops;
  mcdev->owner = THIS_MODULE;

  // After creating cdev, add it to kernel
  ret = cdev_add(mcdev, dev_num, 1);
  if (ret < 0) {
    printk(KERN_ALERT "lcd_driver: unable to add cdev to kernerl\n");
    return ret;
  }

  // Initialize semaphore
  sema_init(&virtual_device.sem, 1);
  msleep(10);

  return 0;
}

// Run when module is uninstalled, unregisters device
static void __exit driver_exit(void) {
  cdev_del(mcdev);
  unregister_chrdev_region(dev_num, 1);

}

// Locks device, sets all GPIOs, and goes through initialization sequence for
// the LCD screens. Return negative on failure, 0 otherwise 
int device_open(struct inode *inode, struct file* filp) {
  if (down_interruptible(&virtual_device.sem) != 0) {
    printk(KERN_ALERT "lcd_driver: could not lock device during open\n");
    return -1;
  }

  // Request access to all the needed GPIO pins
  int res = 0;
  res += gpio_request(DATA_, "Data");
  res += gpio_request(LATCH_, "Latch");
  res += gpio_request(CLOCK_, "Clock");
  res += gpio_request(RS_, "RS1");
  res += gpio_request(RW_, "R/W1");
  res += gpio_request(E_GAME, "E1");
  res += gpio_request(E_SCORE, "E2");

  if (res != 0) {
    printk(KERN_ALERT "lcd_driver: could not access GPIOs during open\n");
    return -1;
  }
	
  // Set all pins for output
  gpio_direction_output(DATA_, 0);
  gpio_direction_output(LATCH_, 0);
  gpio_direction_output(CLOCK_, 0);
  gpio_direction_output(RS_, 0);
  gpio_direction_output(RW_, 0);
  gpio_direction_output(E_GAME, 0);
  gpio_direction_output(E_SCORE, 0);
	
  // Initializes both LCDs
  initialize(GAME_SCREEN);
  initialize(SCORE_SCREEN);
  return 0;
}

// Closes devices, clears displays, frees the GPIO pins, and returns access to semaphore.
int device_close(struct inode* inode, struct  file *filp) {
  up(&virtual_device.sem);
  clearDisplay(GAME_SCREEN);
  displayOff(GAME_SCREEN);
  clearDisplay(SCORE_SCREEN);
  displayOff(SCORE_SCREEN);
  gpio_free(DATA_);
  gpio_free(LATCH_);
  gpio_free(CLOCK_);
  gpio_free(RS_);
  gpio_free(RW_);
  gpio_free(E_GAME);
  gpio_free(E_SCORE);
  return 0;
}

// This function will take the contents of bufSource and print them to the
// LCD screens. The first 16 bytes are printed to the left screen,
// the next 16 bytes are printed to the top row of the right screen,
// and the last 16 bytes are printed to the bottom row of the right screen.
// Any other bytes will be ignored. providing a bufCount of 0 will clear screen 
ssize_t device_write(struct file* filp, const char* bufSource, size_t bufCount, loff_t* curOffset) {
  int firstLine, secondLine, thirdLine;
  int i;

  // Determine how many lines of the displays will be used
  if (bufCount > (CHAR_PER_LINE * NUM_LINES)) {
    firstLine = CHAR_PER_LINE;
    secondLine = CHAR_PER_LINE;
    thirdLine = CHAR_PER_LINE;
  } else if (bufCount > CHAR_PER_LINE * 2) {
    firstLine = CHAR_PER_LINE;
    secondLine = CHAR_PER_LINE;
    thirdLine = bufCount - (CHAR_PER_LINE * 2);
  } else if (bufCount > CHAR_PER_LINE) {
    firstLine = CHAR_PER_LINE;
    secondLine = bufCount - CHAR_PER_LINE;
    thirdLine = 0;
  } else if (bufCount > 0) {
    firstLine = bufCount -CHAR_PER_LINE;
    secondLine = 0;
    thirdLine = 0;
  } else {
    firstLine = 0;
    secondLine = 0;
    thirdLine = 0;
  }

  clearDisplay(GAME_SCREEN);
  clearDisplay(SCORE_SCREEN);

  // Write to the first line of the one line LCD
  for (i = 0; i < firstLine; i++) {
    writeChar(bufSource[i], GAME_SCREEN);
  }

  // Write to the first line of the two line LCD
  for (i = 0; i < secondLine; i++) {
     writeChar(bufSource[i + CHAR_PER_LINE], SCORE_SCREEN);
  }

  // Writes to the second line of the two line LCD
  if (bufCount > CHAR_PER_LINE) setAddress((unsigned char) 0x40, SCORE_SCREEN);
  for (i = 0; i < thirdLine; i++) {
    writeChar(bufSource[i + (2*CHAR_PER_LINE)], SCORE_SCREEN);
  }

  // returns cursor of one line to far left
  setAddress((unsigned char) 0x00, GAME_SCREEN);

  return copy_from_user(virtual_device.data, bufSource, bufCount);
}

// Initializes the LCD with the proper series of commands
void initialize(int screenSel) {
  gpio_set_value(RS_, 0);
  gpio_set_value(RW_, 0);
  msleep(15);

  // Function Set #1
  command((unsigned char) 0x30, screenSel);
  msleep(5);

  // Function Set #2
  lcdSend(screenSel);
  msleep(1);
	
  // Function Set #3
  lcdSend(screenSel);
  msleep(1);

  // Function Set #4
  if (screenSel == SCORE_SCREEN) {
    command((unsigned char) 0x38, screenSel);  // 5x7 font, 2 lines
  } else {
    command((unsigned char) 0x34, screenSel);  // 5x10 font, 1 line
  }
  msleep(1);

  // Display OFF
  command((unsigned char) 0x08, screenSel);
  udelay(50);

  // Clear Display
  command((unsigned char) 0x01, screenSel);
  msleep(16);

  // Increment mode	
  command((unsigned char) 0x06, screenSel);
  udelay(50);

  // Entry mode set
  if (screenSel == SCORE_SCREEN) {
    command((unsigned char) 0x0C, screenSel);  // Cursor OFF, Blink OFF
  } else {
    command((unsigned char) 0x0F, screenSel);  // Cursor ON, Blink ON
  }
  udelay(50);
}

// Loads data through the shift register and sends the command to the LCD
void command(unsigned char data, int screenSel) {
  setBus(data);
  lcdSend(screenSel);
}

// Loads and sends data from the shift register
void setBus(unsigned char num) {
  int i = 7;
  int j = 0;
  int binary[8];

  // Building the binary version of num
  if (j = 0; j < 8; j++) {
    binary[j] = num % 2;
    num = num >> 1;
  }

  // Inserting binary value into shift register
  if (i = 7; i >= 0; i--) { 
    gpio_set_value(DATA_, binary[i]);
    gpio_set_value(CLOCK_, 1);
    udelay(10);
    gpio_set_value(CLOCK_, 0);
    i--;
  }

  gpio_set_value(LATCH_, 1);
  udelay(50);
  gpio_set_value(LATCH_, 0);

}

// Clears the LCD selected by screenSel
void clearDisplay(int screenSel){
  gpio_set_value(RS_, 0);
  gpio_set_value(RW_, 0);
  command ((unsigned char) 0x01, screenSel); // Clear Display
  msleep(16);
}

// Turns the LCD selected by screenSel off
void displayOff(int screenSel) {
  gpio_set_value(RS_, 0);
  gpio_set_value(RW_, 0);
  command((unsigned char) 0x08, screenSel); // Display OFF
  udelay(50);
}

// Sets the R/W pointer to the address specified of the screenSel screen
void setAddress(unsigned char address, int screenSel) {
  gpio_set_value(RS_, 0);
  gpio_set_value(RW_, 0);
  address |= 0x80;
  setBus(address);
  lcdSend(screenSel);
  udelay(50);
}

// Sets DB7 to DB0 to the given 8 bits of the screen set by screenSel
void writeChar(unsigned char character, int screenSel) {
  gpio_set_value(RS_, 1);
  gpio_set_value(RW_, 0);
  setBus(character);
  lcdSend(screenSel);
  udelay(50);
}

// Flips the enable switch on the LCD to execute the loaded instruction
// of the screen set by screenSel
void lcdSend(int screenSel) {
  gpio_set_value(EArr[screenSel], 1);
  udelay(50);
  gpio_set_value(EArr[screenSel], 0);
}

MODULE_LICENSE("GPL"); // module license: required to use some functionalities.
module_init(driver_entry); // declares which function runs on init.
module_exit(driver_exit);  // declares which function runs on exit.

