/*
 * buttonHero.h: contains includes, method declerations, and variables
 *
 * Brad Marquez
 * Aigerim Shintemirova
 * Joseph Rothlin
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#define SCREEN_SIZE 16 // total length of a line on the LCD screen
#define WRONG_GUESSES 8 // has to be less than or equal to 8
#define DELAY_TIME 1000 // time between each input update in us
#define NEW_LCD_DIR "/dev/lcd_driver" // lcd driver directory
#define NEW_BUT_DIR "/dev/button_driver" // button driver directory

// Constants associated with button controls
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define PRESS 4
#define NUM_BUTTONS 5

// Note frequencies for buzzer
const int noteG = 2551020;
const int noteAb = 2409639;
const int noteA = 2272727;
const int noteBb = 2145186;
const int noteB = 2024783;
const int noteC = 1912046;
const int noteD = 1702620;
const int noteE = 1517451;

// Static variables for file communication
static int fd_lcd = 0, fd_but = 0;
static FILE *sys2 = NULL, *dirduty = NULL, *dirT = NULL;

// Pauses program until any button input is received
void pressAnyButton();

// Brings up quit screen on LCD, returns 1 if user chose to quit, 0 otherwise
int wantToQuit();

// Interupt handler, makes all necessary closing steps to terminate program
// when the SIGINT signal is received
void sigHandler(int signo);

// Takes in two integers which represent the score the player received in the
// current game and the previous high score. This function will set the screen
// to the correct value and return the new high score.
int printLose(int currentScore, int highScore);

// Calls open funtion of the lcd driver and button driver setting the static
// variables fd_lcd and fd_but to their correct values
void openPath(void);

// Prints the instructions of the game to terminal
void instructions(void);

// Initiates the game
void playGame(void);

// Plays the given note on the buzzer
void buzzer(int note, int count);

// Plays the sound sequence for a lose
void loseMusic(void);

// Plays the sound sequence for a win
void winMusic(void);

// Closes the files associated with the buzzer
void closeBuzzer(void);
