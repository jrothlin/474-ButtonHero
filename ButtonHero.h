/*
 * ButtonHero.h
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
 *
 * This file contains include files and function/variable delerations
 * used in ButtonHero.c
 */

#ifndef _BUTTONHERO_H_
#define _BUTTONHERO_H_

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
#include <stdbool.h>

#define SCREEN_SIZE 16  // total length of a line on the LCD screen
#define WRONG_GUESSES 8  // has to be less than or equal to 8
#define DELAY_TIME 1000  // time between each input update in microseconds
#define NEW_LCD_DIR "/dev/lcd_driver"  // lcd driver directory
#define NEW_BUT_DIR "/dev/button_driver"  // button driver directory

// Constants associated with button controls
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define PRESS 4
#define NONE 5
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

// Set of game play symboles
const char symbols[NUM_BUTTONS + 1] = {'^', 'v', '<', '>', 'o', ' '};

// Static variables for file communication
static int fd_lcd = 0, fd_but = 0;
static FILE *pwm = NULL, *dirDuty = NULL, *dirT = NULL;

// State of the game, there is only one game during the life of the program
typedef struct game_state_struct {
  int highScore;
  int inputs[NUM_BUTTONS];
  bool quit;
} game_state;

// Contains the state of the session, a session is the point from when the
// symbols begin to scroll across the screen until the moment the user obtains
// too many misses. There can be several session in a single game depending
// on how many times the user chooses to play through a session
typedef struct session_state_struct {
  int currScore;
  int counter;
  int pressed;
  int misses;
  int inputted;
  int correctInput;
} session_state;

// Pauses program until any button input is received
void pressAnyButton();

// Brings up quit screen on LCD, returns 1 if user chose to quit, 0 otherwise
int wantToQuit();

// Interupt handler, makes all necessary closing steps to terminate program
// when the SIGINT signal is received
void shutDown();

// Takes in two integers which represent the score the player received in the
// current game and the previous high score. This function will set the screen
// to the correct display and if currentScore is greater than highScore,
// highScore will be modified and set to be equal to currentScore.
void printGameOver(int currentScore, int *highScore);

// Calls open funtion of the lcd driver and button driver setting the static
// variables fd_lcd and fd_but to their correct values
void openPath(void);

// Prints the instructions of the game to terminal
void instructions(void);

// Plays the given note on the buzzer
void buzzer(int note);

// Plays the sound sequence for a lose
void loseMusic(void);

// Plays the sound sequence for a win
void winMusic(void);

// Prints the next screen frame to the LCD screens. The symbolScreen will be
// modified by shift all of it's character to the left by one space (except for
// a terminating null character at the end) and filling
// in the empty space with a random character from the symbols array.
// Return -1 on error, 0 on success.
// **WARNING** this function expects the length of symbolScreen to be
// SCREEN_SIZE + 1. Providing a string with any other length will produce
// undefined behavior.
int nextScreenFrame(const session_state *currSession, char *symbolScreen);

// Updates the currSession struct based on the current state of the game. The
// direction argument is the character which the played must currently press
// to get the point.
void updateSession(session_state *currSession, char direction);

#endif  // _BUTTONHERO_H_
