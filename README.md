Lab 3: Introduction to Kernel Modules\n
EE 474 Spring 2016\n
Contributors: Brad Marquez, Joseph Rothlin, Aigerim Shintemirova\n

## Synopsys
The purpose of this lab is to further develop our embedded Linux skills by
creating a Kernel module that replaces our user space code from previous labs
which communicated with various periferal devices. In this lab we
created a ButtonHero game (synonymous to Guitar Hero) using two periferal LCD
screens as the disply, a shift register for injecting data into the LCDs, a
5-way directional button as the control, and a piezzo buzzer for sound effects.
This project includes a kernel space driver for the button, a kernel space
driver for the shift register/LCD setup, and a main user space program
executing the game. The target device for this project is the BeagleBone
Black with the ARM Cortex-A8 processor onboard.

## Code Usage
1. With the Makefile use the "make all" command to cross compile the code
2. Again, with the Makefile use the "make transfer" command to transfer all
   the necessary file to the BeagleBone
3. SSH protocol is used to connect to the Beaglebone, use the command
   ssh root@192.168.7.2 to move to the BeagleBone
4. Use "insmod <filename>" to insert both of the .ko files into the Linux OS
   (You can use lsmod to make sure the modules were properly insert)
5. Use "dmesg tail" to display the messages from the inserted modules
6. Each module should have printed a line instructing how to create their
   device files. Use the quoted command to create the device file (one for
   each module) which will look something like "mknod /dev/<string> c # 0".
7. Congratulation! Now the software should be ready to go! Just run the
   ButtonHero executable to play!
	
## Notes - lcd\_driver.c
This file acts as the single driver for both LCDs. Using the system open/close
functions the user can intialize and uninitialize the LCD screens and GPIO
pins. Using the system write call the user can provide up to 48 characters
toimmediatly be printed to the two screens. Using the seperate enable
pins for the LCDs this module can load the data to write to the shift
register and then choose which screen to write to by selecting which
enable pin to toggle.

## Notes - button\_driver.c
This file acts as the driver for the 5-way button. Using the system open/close
functions the user can initialize and uninitialize the GPIO pins associated
with the button. Using the system read call the user can receive 20 bytes
(5 integers) of data which contains the status of each direction on the button.

## Notes - buttonHero.c
This file was written in user space C and uses both modules above to
communicate with periferal devices while executing the game. This level
of abstraction made communicating with periferals fairly simple from
the perspective of ButtonHero.c. This program creates the visual effect
of scrolling arrows across a screen during which the user is meant to
press the corresponding button direction assosiated with that arrow. High
scores are recorded and displayed at the end of each round, and a buzzer
along with LEDs were added (and controlled by buttonHero.c) in order to
increase the aesthetics of the game.
