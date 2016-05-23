/*
 * lcd_driver.c: holds a buffer of 100 characters as device file.
 *             prints out contents of buffer on read.
 *             writes over buffer values on write, writing to LCD screens.
 *
 * Brad Marquez
 * Aigerim Shintemirova
 * Joseph Rothlin
 * 
 */

#include "lcd_driver.h"

/********************* FILE OPERATION FUNCTIONS ***************/

// runs on startup
// intializes module space and declares major number.
// assigns device structure data.
static int __init driver_entry(void) {
	// REGISTERIONG OUR DEVICE WITH THE SYSTEM
	// ALLOCATE DYNAMICALLY TO ASSIGN OUR DEVICE
	int ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		printk(KERN_ALERT "lcd_driver: Failed to allocate a major number\n");
		return ret;
	}
	printk(KERN_INFO "lcd_driver: major number is %d\n", MAJOR(dev_num));
	printk(KERN_INFO "Use mknod /dev/%s c %d 0 for device file\n", DEVICE_NAME, MAJOR(dev_num));

	// CREATE CDEV STRUCTURE, INITIALIZING CDEV
	mcdev = cdev_alloc();
	mcdev->ops = &fops;
	mcdev->owner = THIS_MODULE;

	// After creating cdev, add it to kernel
	ret = cdev_add(mcdev, dev_num, 1);
	if (ret < 0) {
		printk(KERN_ALERT "lcd_driver: unable to add cdev to kernerl\n");
		return ret;
	}

	// Initialize SEMAPHORE
	sema_init(&virtual_device.sem, 1);
	msleep(10);

	return 0;
}

// called up on exit.
// unregisters the device and all associated gpios with it.
static void __exit driver_exit(void) {
	cdev_del(mcdev);
	unregister_chrdev_region(dev_num, 1);

}

// Called on device file open
// inode reference to file on disk, struct file represents an abstract
// checks to see if file is already open (semaphore is in use)
// prints error message if device is busy.
int device_open(struct inode *inode, struct file* filp) {
	if (down_interruptible(&virtual_device.sem) != 0) {
		printk(KERN_ALERT "lcd_driver: could not lock device during open\n");
		return -1;
	}

	// Request access to all the needed GPIO pins
	gpio_request(DATA_, "Data");
	gpio_request(LATCH_, "Latch");
	gpio_request(CLOCK_, "Clock");
	gpio_request(RS_, "RS1");
	gpio_request(RW_, "R/W1");
	gpio_request(E_GAME, "E1");
	gpio_request(E_SCORE, "E2");
	
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

// Called upon close
// closes devices, clears displays, frees the GPIO pins, and returns access to semaphore.
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

// Called when user wants to get info from device file
ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset) {
	return copy_to_user(bufStoreData, virtual_device.data, bufCount);
}

// Called when user wants to send info to device
// Calling a shift register file to write to both LCDs
ssize_t device_write(struct file* filp, const char* bufSource, size_t bufCount, loff_t* curOffset) {
	int firstLine, secondLine, thirdLine, valid = 1;
	int i;
	
	// Determine how many lines of the displays will be used
	if (bufCount > (CHAR_PER_LINE * NUM_LINES)) {
		firstLine = CHAR_PER_LINE;
		secondLine = CHAR_PER_LINE;
		thirdLine = CHAR_PER_LINE; //
	} else if (bufCount > CHAR_PER_LINE * 2) {
		firstLine = CHAR_PER_LINE;
		secondLine = CHAR_PER_LINE;
		thirdLine = bufCount - (CHAR_PER_LINE * 2);
	} else if (bufCount > CHAR_PER_LINE) {
		firstLine = CHAR_PER_LINE;
		secondLine = bufCount - CHAR_PER_LINE;
		thirdLine = 0;
	} else {
		valid = 0;
	}

	if (valid) {
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
	}
	return copy_from_user(virtual_device.data, bufSource, bufCount);
}

// Initializes the LCD with the proper series of commands
void initialize(int screenSel) {
	gpio_set_value(RS_, 0);
	gpio_set_value(RW_, 0);
	msleep(15);

	command((unsigned char) 0x30, screenSel); // Function Set #1
	msleep(5);

	lcdSend(screenSel); // Function Set #2
	msleep(1);
	
	lcdSend(screenSel); // Function Set #3
	msleep(1);

	if (screenSel == SCORE_SCREEN) { // Function Set #4
		command((unsigned char) 0x38, screenSel); // 5x7 font, 2 lines
	} else {
		command((unsigned char) 0x34, screenSel); // 5x10 font, 1 line
	}
	msleep(1);

	command((unsigned char) 0x08, screenSel); // Display OFF
	udelay(50);

	command((unsigned char) 0x01, screenSel); // Clear Display
	msleep(16);
	
	command((unsigned char) 0x06, screenSel); // Increment mode
	udelay(50);

	if (screenSel == SCORE_SCREEN) {// Entry Mode Set
		command((unsigned char) 0x0C, screenSel); // Cursor OFF, Blink OFF
	} else {
		command((unsigned char) 0x0F, screenSel); // Cursor ON, Blink On
	}
	udelay(50);
}

// Loads data through the shift register and sends the command to the LCD
void command(unsigned char data, int screenSel) {
	printk("command 1\n");
	setBus(data);
	printk("command 2\n");
	lcdSend(screenSel);
	printk("command 3\n");
}

// Loads and sends data into and from the shift register
void setBus(unsigned char num) {
	int i = 7;
	int j = 0;
	int binary[8];
	int temporary = num;

	// Building the binary version of num
	while (j < 8) {
		binary[j] = temporary % 2;
		temporary = temporary >> 1;
		j++;
	}

	// Inserting binary E0_value into shift register
	while (i >= 0) {
		gpio_set_value(DATA_, binary[i]);  // Set the data line to the next value

		// Toggle the clock
		gpio_set_value(CLOCK_, 1);
		udelay(10);
		gpio_set_value(CLOCK_, 0);
		i--;
	}

	// Toggle the latch
	gpio_set_value(LATCH_, 1);
	udelay(50);
	gpio_set_value(LATCH_, 0);

}

// Clears the LCD
void clearDisplay(int screenSel){
	gpio_set_value(RS_, 0);
	gpio_set_value(RW_, 0);
	command ((unsigned char) 0x01, screenSel); // Clear Display
	msleep(16);
}

// Turns the LCD off
void displayOff(int screenSel) {
	gpio_set_value(RS_, 0);
	gpio_set_value(RW_, 0);
	command((unsigned char) 0x08, screenSel); // Display OFF
	udelay(50);
}

// Sets the R/W pointer to the address specified
void setAddress(unsigned char address, int screenSel) {
	gpio_set_value(RS_, 0);
	gpio_set_value(RW_, 0);
	address |= 0x80;
	setBus(address);
	lcdSend(screenSel);
	udelay(50);
}

// Sets DB7 to DB0 to the given 8 bits
void writeChar(unsigned char character, int screenSel) {
	gpio_set_value(RS_, 1);
	gpio_set_value(RW_, 0);
	setBus(character);
	lcdSend(screenSel);
	udelay(50);
}

// Flips the enable switch on the LCD to execute the loaded instruction
void lcdSend(int screenSel) {
	printk("Pre enable high\n");
	gpio_set_value(EArr[screenSel], 1);	// flip enable high
	udelay(50);
	printk("Pre enable low\n");
	gpio_set_value(EArr[screenSel], 0); // sends on falling edge
	printk("Post enable low\n");
}

MODULE_LICENSE("GPL"); // module license: required to use some functionalities.
module_init(driver_entry); // declares which function runs on init.
module_exit(driver_exit);  // declares which function runs on exit.

