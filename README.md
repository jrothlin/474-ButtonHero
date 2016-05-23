Lab 3: Introduction to Kernel Modules
EE 474 Spring 2016
Workstation #12
Contributors: Brad Marquez, Joseph Rothlin, Aigerim Shintemirova

## Synopsys

The purpose of this lab is to further develop our embedded Linux skills by
creating a Kernel module that replaces our user space code from Lab2 which
communicated with various periferal devices and creating. In this lab we
created a buttonHero game using a periferal LCD screen as the disply
and a 5-way directional button as the control. Our code consisted of a
LKM which acted as the driver for two LCD screens with a shift register
for GPIO conservation, a LKM which acted as the driver for the 5-way
button, and the user space code which executed the buttonHero game.

## Code Execution
1. SSH protocol is used to operate the Beaglebone
2. A shift register was used to transfer data from the Beaglebone to the LCD
   screens in order to conserve GPIO pins.
3. In order to transfer the file to the board use command:
	scp Hangman root@192.168.7.2:~
4. To access the pins we imported linux/gpio.h and used the commands within
   that file in our kernel modules.
	
## Notes - lcd\_driver.c
This file acted as the single driver for both LCDs. Using the system open/close
functions the user could intialize and uninitialize the LCD screens and GPIO
pins. Using the system write call the user could provide up to 48 characters
which would then be printed to the two screens. Using the seperate enable
pins for the LCDs this module could load the data to write to the shift
register and then choose which screen to write to by selecting which
enable pin to toggle.

## Notes - button\_driver.c
This file acted as the driver for the 5-way button. Using the system open/close
functions the user could initialize and uninitialize the GPIO pins associated
with the button. Using the system read call the user can receive 20 bytes
of data which contains the status of each direction on the button.

## Notes - buttonHero.c
This file was written in user space C and used both the modules above to
communicate with periferal devices while executing the game. This level
of abstraction made communicating with periferals fairly simple from
the perspective of buttonHero.c. This program creates the visual effect
of scrolling arrows across a screen during which the user is meant to
press the corresponding button direction assosiated with that arrow. High
scores are recorded and displayed at the end of each round, and a buzzer
along with LEDs were added (and controlled by buttonHero.c) in order to
increase the aesthetics of the game.
