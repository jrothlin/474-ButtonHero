/*
 * ButtonHero.c
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
 *
 * This program plays a game synonymous to "Guitar Hero: entitled "Button Hero"
 * Symbols scroll across a screen as the user attempt to press the
 * corresponding buttons at the correct time. This program is targeted for
 * the ARM-Cortex processor aboard the Beablebone Black and uses two kernal
 * module drivers (lcd_driver and button_driver) to interface with the hardware
 * used for this game.
 */

#include "ButtonHero.h"

int main() {
  // Initial setup
  signal(SIGINT, shutDown);
  openPath();
  instructions();
  srand(time(NULL));
  game_state currGame;
  memset(&currGame, 0, sizeof(game_state));

  // Sets up piezobuzzer for sound using designated PWM pin
  pwm = fopen("/sys/devices/bone_capemgr.9/slots", "w");
  fseek(pwm, 0, SEEK_END);
  fprintf(pwm, "am33xx_pwm");
  fprintf(pwm, "bone_pwm_P9_14");
  fflush(pwm);

  // Sets the pointers to the appropriate duty and period files
  dirDuty = fopen("/sys/devices/ocp.3/pwm_test_P9_14.15/duty", "w");
  dirT = fopen("/sys/devices/ocp.3/pwm_test_P9_14.15/period", "w");

  // Main game loop
  while (!currGame.quit) {
    char *playScreen = "                Press button    to start!       ";
    write(fd_lcd, playScreen, SCREEN_SIZE * 3);
    pressAnyButton();
    // usleep necessary here to prevent signal overlap once game begins
    usleep(500000);

    // Initializes the playing screen
    char symbolScreen[SCREEN_SIZE + 1];
    int i;
    for (i = 0; i < SCREEN_SIZE; i++) {
      symbolScreen[i] = ' ';
    }
    symbolScreen[SCREEN_SIZE] = '\0';

    session_state currSession;
    memset(&currSession, 0, sizeof(session_state));
    currSession.misses = -1;

    // Current game session loop
    while (currSession.misses < WRONG_GUESSES){
      // When software counter resets, update to the next screen frame
      if (currSession.counter == 0) {
        // Respond to player's input
        if (currSession.correctInput && symbolScreen[0] != ' ') {
          currSession.currScore++;
        } else if (!currSession.correctInput) {
          currSession.misses++;
        }

        currSession.inputted = 0;
        currSession.correctInput = 0;

        if (nextScreenFrame(&currSession, symbolScreen) == -1) {
          shutDown();
          return EXIT_FAILURE;
        }
      }
      // Delay inbetween input update
      usleep(DELAY_TIME);

      // Reads the current status of the button inputs
      read(fd_but, currGame.inputs, NUM_BUTTONS * sizeof(int));
      currSession.pressed = 5;
      for (i = 0; i < NUM_BUTTONS; i++) {
        if (currGame.inputs[i] == 1) {
          currSession.pressed = i;
        }
      }

      // Play sound on buzzer which corresponds to the input
      // This is done less often to improve smoothness of gameplay
      if (currSession.counter % 15 == 0) {
        if (currSession.pressed == 0) {
          buzzer(noteA);
        } else if (currSession.pressed == 1) {
          buzzer(noteB);
        } else if (currSession.pressed == 2) {
          buzzer(noteC);
        } else if (currSession.pressed == 3) {
          buzzer(noteD);
        } else if (currSession.pressed == 4) {
          buzzer(noteE);
        } else {
          buzzer(0);
        }
      }

      updateSession(&currSession, symbolScreen[0]);
    }    
    printGameOver(currSession.currScore, &(currGame.highScore));
    pressAnyButton();
    usleep(500000);

    // Prompts user to choose to play again
    currGame.quit = wantToQuit();
    usleep(500000);
  }
  shutDown();
  return EXIT_SUCCESS;
}

// Plays short tune to signal a player loss
void loseMusic() {
  buzzer(noteBb);
  usleep(500000);
  buzzer(noteA);
  usleep(500000);
  buzzer(noteAb);
  usleep(500000);
  buzzer(noteG);
  usleep(1000000);
  buzzer(0);
}

// Plays short tune to signal a player win
void winMusic() {
  buzzer(noteA);
  usleep(500000);
  buzzer(noteBb);
  usleep(500000);
  buzzer(noteB);
  usleep(500000);
  buzzer(noteC);
  usleep(1000000);
  buzzer(0);
}

// Plays given sound on the buzzer
void buzzer(int note) {
  fprintf(dirT, "%d", note);
  fflush(dirT);
  fprintf(dirDuty, "%d", note / 2);
  fflush(dirDuty);
}

// Opens files used to interface with kernal drivers for LCDs and 5-way button
void openPath(){
  fd_lcd = open(NEW_LCD_DIR, O_RDWR);
  fd_but = open(NEW_BUT_DIR, O_RDWR);
  if (fd_lcd < -1 || fd_but < -1) {
    if (fd_lcd < -1) {
      printf("File %s cannot be opened: %s\n", NEW_LCD_DIR, strerror(errno));
      exit(1);
    } else {
      printf("File %s cannot be opened: %s\n", NEW_BUT_DIR, strerror(errno));
      exit(1);
    }
  }
}

// Prints the game session ending screen
void printGameOver(int currentScore, int *highScore) {
  bool lost;
  char overScreen[SCREEN_SIZE * 3];
  if (currentScore > *highScore) {
    lost = false;
    *highScore = currentScore;
    strcpy(overScreen, "NEW HIGH SCORE! ");
  } else {
    lost = true;
    strcpy(overScreen, "SORRY, YOU LOST!");
  }

  // Builds string for current score
  char scoreString[SCREEN_SIZE + 1];
  sprintf(scoreString, "Score: %d", currentScore);
  int i;
  for (i = strlen(scoreString); i < SCREEN_SIZE; i++) {
    scoreString[i] = ' ';
  }
  scoreString[SCREEN_SIZE] = '\0';
  strcat(overScreen, scoreString);

  // Builds string for current high score
  char highString[SCREEN_SIZE + 1];
  sprintf(highString, "High Score: %d", *highScore);
  for (i = strlen(highString); i < SCREEN_SIZE; i++) {
    highString[i] = ' ';
  }
  highString[SCREEN_SIZE] = '\0';
  strcat(overScreen, highString);

  write(fd_lcd, overScreen, SCREEN_SIZE * 3);
  if (lost) {
    loseMusic();
  } else {
    winMusic();
  }	
}

// Closes all files for exit when termination signal it received
void shutDown() {
  buzzer(0);
  if (pwm != NULL) fclose(pwm);
  if (dirDuty != NULL) fclose(dirDuty);
  if (dirT != NULL) fclose(dirT);
  if (fd_lcd != 0) close(fd_lcd);
  if (fd_but != 0) close(fd_but);
}

// Prints the instructions for the user to view on the terminal on game start up
void instructions(){
  printf("\nHello! Welcome to Button Hero!\n\nINSTRUCTIONS: Playing this ");
  printf("game requires one user. Press the corresponding\n");
  printf("button when it gets to the far left of the single lined screen. ");
  printf("Current score\nand the number of misses are displayed ");
  printf("on the two-lined LCD screen. A miss is\n");
  printf("given on a wrong input or when the user misses an input. ");
  printf("The user is allowed %d\nmisses", WRONG_GUESSES);
  printf(" until they lose. The current high score is then displayed to ");
  printf("the user\nand the user is prompted to play again.\n");

  printf("\nTypes of Notes:\n");
  printf("1. ^ = up on the joystick\n");
  printf("2. v = down on the joystick\n");
  printf("3. < = left on the joystick\n");
  printf("4. > = right on the joystick\n");
  printf("5. o = press the joystick\n");
  printf("6.   = do nothing\n");	
}

// Waits for the user to input anything on the button
void pressAnyButton() {
  int input[NUM_BUTTONS] = {0, 0, 0, 0, 0};
  while (!(input[UP] ||
           input[DOWN] ||
           input[LEFT] ||
           input[RIGHT] ||
           input[PRESS])) {
    read(fd_but, input, NUM_BUTTONS * sizeof(int));
  }
}

// Asks user if they would like to play again, returns 1 for yes and 0 for no
int wantToQuit() {
  char quitScreen[SCREEN_SIZE * 3] = "Play again?      >No   Yes                      ";
  write(fd_lcd, quitScreen, SCREEN_SIZE * 3);
  int input[NUM_BUTTONS] = {0, 0, 0, 0, 0};	
  bool cursorOnNo = true;

  // Wait until an option is selected by pressing the button
  while (input[4] != 1) {
    read(fd_but, input, (NUM_BUTTONS * sizeof(int)));

    // If the cursor is moved to the right while it points to "No"
    if ((cursorOnNo == 1) && (input[RIGHT] == 1)) {
      cursorOnNo = false;
      quitScreen[17] = ' ';
      quitScreen[22] = '>';
      write(fd_lcd, quitScreen, SCREEN_SIZE * 3);

    // If the cursor is moved to the left while it points to "Yes"
    } else if ((cursorOnNo == 0) && (input[LEFT] == 1)) {
      cursorOnNo = true;
      quitScreen[17] = '>';
      quitScreen[22] = ' ';
      write(fd_lcd, quitScreen, SCREEN_SIZE * 3);
    }
  }
  return cursorOnNo;
}

// Prints the next screen frame to the LCD screens. The symbolScreen will be
// modified by shift all of it's character to the left by one space (except for
// a terminating null character at the end) and filling
// in the empty space with a random character from the symbols array.
// Return -1 on error, 0 on success.
// **WARNING** this function expects the length of symbolScreen to be
// SCREEN_SIZE + 1. Providing a string with any other length will produce 
// undefined behavior.
int nextScreenFrame(session_state *currSession, char *symbolScreen) {
  // Shift symbols
  int i;
  for (i = 0; i < SCREEN_SIZE - 1; i++) {
    symbolScreen[i] = symbolScreen[i + 1];
  } 

  // Add random note to game screen
  symbolScreen[SCREEN_SIZE - 1] = symbols[rand() % 6];
  symbolScreen[SCREEN_SIZE] = '\0';

  // Builds string for current score display
  char scoreString[SCREEN_SIZE + 1];
  sprintf(scoreString, "Score: %d", currSession->currScore);
  for (i = strlen(scoreString); i < SCREEN_SIZE; i++) {
    scoreString[i] = ' ';
  }
  scoreString[SCREEN_SIZE] = '\0';

  // Builds string for current misses display
  char missMarks[SCREEN_SIZE + 1];
  strcpy(missMarks, "Misses: ");
  for (i = SCREEN_SIZE - WRONG_GUESSES; i < (SCREEN_SIZE - WRONG_GUESSES + currSession->misses); i++) {
    missMarks[i] = 'X';
  }
  for (i = (SCREEN_SIZE - WRONG_GUESSES + currSession->misses); i < SCREEN_SIZE; i++) {
    missMarks[i] = ' ';
  }
  missMarks[SCREEN_SIZE] = '\0';

  // Builds total string that is passed to the LCD driver
  char total[SCREEN_SIZE * 3];
  strcpy(total, symbolScreen);
  strcat(total, scoreString);
  strcat(total, missMarks);

  write(fd_lcd, total, SCREEN_SIZE * 3);
  return 0;
}

// Updates the currSession struct based on the current state of the game. The
// direction argument is the character which the played must currently press
// to get the point.
void updateSession(session_state *currSession, char direction) {
  // The input matches the note and it's the first input we've received
  if (symbols[currSession->pressed] == direction && currSession->pressed != 5 && !currSession->inputted) {
    currSession->correctInput = 1;
    currSession->inputted = 1;

  // There is an input and it is wrong
  } else if (currSession->pressed != 5 && (symbols[currSession->pressed] != direction || direction == ' ')) {
    currSession->correctInput = 0;
    currSession->inputted = 1;

  // No symbol is showing and we received no input
  } else if (direction == ' ' && currSession->pressed == 5 && !currSession->inputted) {
    currSession->correctInput = 1;
  }

  // Increase speed of the game as score rises
  if (currSession->currScore < 250) {
    currSession->counter = (currSession->counter + 1) % (750 - currSession->currScore * 3);
  } else {
    currSession->counter = (currSession->counter + 1) % 2;  // Cap on difficulty increase
  }
}
