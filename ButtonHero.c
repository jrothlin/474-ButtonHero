/*
 * ButtonHero.c
 *
 * Brad Marquez
 * Joseph Rothlin
 * Aigerim Shintemirova
 * 23/5/2016
 *
 * This program plays a game synonymous to "Guitar Hero: entitled "Button Hero"
 * Symbols scroll across a screen as the user attempt to press the
 * corresponding buttons at the correct time. This program is targeted for
 * the ARM-Cortex processor and uses two kernal module drivers (lcd_driver
 * and button_driver) to interface with the hardware used for this game.
 */

#include "ButtonHero.h"

int main() {
  // Signal handler for termination signal from terminal
  signal(SIGINT, sigHandler);
  openPath();
  instructions();
  playGame();
  close(fd_lcd);
  close(fd_but);
  return 0;
}

// Initiates main game loop for "Button Hero"
void playGame(){
  srand(time(NULL));
  int highScore = 0;
  int quit = 0;
  int inputs[NUM_BUTTONS] = {0, 0, 0, 0, 0};

  // Sets up piezobuzzer for sound using designated PWM pin
  pwm = fopen("/sys/devices/bone_capemgr.9/slots", "w");
  fseek(pwm, 0, SEEK_END);
  fprintf(pwm, "am33xx_pwm");
  fprintf(pwm, "bone_pwm_P9_14");
  fflush(pwm);

  // Sets the pointers to the appropriate duty and period files
  dirduty = fopen("/sys/devices/ocp.3/pwm_test_P9_14.15/duty", "w");
  dirT = fopen("/sys/devices/ocp.3/pwm_test_P9_14.15/period", "w");

  // Main game loop
  while (!quit) {
    // Splash screen before game start
    char *playScreen = "                Press button    to start!       ";
    write(fd_lcd, playScreen, SCREEN_SIZE * 3);
    pressAnyButton();
    usleep(500000);

    // Initializes the playing screen
    char screen[SCREEN_SIZE + 1];
    int i;
    for (i = 0; i < SCREEN_SIZE; i++) {
      screen[i] = ' ';
    }
    screen[SCREEN_SIZE] = '\0';

    int noteType = 5;
    int inputted, index;
    // Current game session loop
    while (misses < WRONG_GUESSES){
      // When software counter resets, update to the next screen frame
      if (counter == 0) {
        // Respond to player's input
        if (rightInput && screen[0] != ' ') {
          currentScore++;
        } else if (!rightInput) {
          misses++;
        }

        rightInput = 0;
        inputted = 0;
        noteType = rand() % 6;

        // Shift symbols
        for (i = 0; i < SCREEN_SIZE - 1; i++) {
          screen[i] = screen[i + 1];
        } 

        // Add random note to game screen
        if (noteType == 0) {
          screen[SCREEN_SIZE - 1] =  '^'; // up arrow 
        } else if (noteType == 1) {
          screen[SCREEN_SIZE - 1] =  'v'; // down arrow 
        } else if (noteType == 2) {
          screen[SCREEN_SIZE - 1] =  '<'; // left arrow 
        } else if (noteType == 3) {
          screen[SCREEN_SIZE - 1] =  '>'; // right arrow 
        } else if (noteType == 4) {
          screen[SCREEN_SIZE - 1] =  'o'; // press button
        } else {
          screen[SCREEN_SIZE - 1] =  ' '; // space = no input
        }

        // Builds string for current score display
        char scoreString[17];
        sprintf(scoreString, "Score: %d", currentScore);
        for (i = strlen(scoreString); i < SCREEN_SIZE; i++) {
          scoreString[i] = ' ';
        }
        scoreString[SCREEN_SIZE] = '\0';

        // Builds string for current misses display
        char missMarks[17];
        strcpy(missMarks, "Misses: ");
        for (i = SCREEN_SIZE - WRONG_GUESSES; i < (SCREEN_SIZE - WRONG_GUESSES + misses); i++) {
          missMarks[i] = 'X';
        }
        for (i = (SCREEN_SIZE - WRONG_GUESSES + misses); i < SCREEN_SIZE; i++) {
          missMarks[i] = ' ';
        }
        missMarks[SCREEN_SIZE] = '\0';

        // Builds total string that is passed to the LCD driver
        char total[SCREEN_SIZE * 3];
        strcpy(total, screen);
        strcat(total, scoreString);
        strcat(total, missMarks);

        // Prints onto LCDs
        write(fd_lcd, total, SCREEN_SIZE * 3);

        // Prints screen, score, and misses to terminal
        for (i = 0; i < strlen(total); i++) {
          if (i % 16 == 0) {
            printf("\n");
          }
          printf("%c",total[i]);
        }
        printf("\n");			
      }
      // Delay inbetween input update
      usleep(DELAY_TIME);

      // Reads the current status of the button inputs
      read(fd_but, inputs, NUM_BUTTONS * sizeof(int));
      index = 5;
      for (i = 0; i < NUM_BUTTONS; i++) {
        if (inputs[i] == 1) {
          index = i;
        }
      }

      // Processes input, plays corresponding sound on buzzer
      char note;
      if (counter % 15 == 0) {
        if (index == 0) {
          note =  '^'; // up arrow
          buzzer(noteA);
        } else if (index == 1) {
          note =  'v'; // down arrow 
          buzzer(noteB);
        } else if (index == 2) {
          note =  '<'; // left arrow 
          buzzer(noteC);
        } else if (index == 3) {
          note =  '>'; // right arrow 
          buzzer(noteD);
        } else if (index == 4) {
          note = 'o'; // press button
          buzzer(noteE);
        } else {
          note =  ' '; // space = no input
          buzzer(0);
        }

      // The input matches the note and it's the first input we've received
      if (note == screen[0] && index != 5 && !inputted) {
        rightInput = 1;
        inputted = 1;
      // There is an input and it is wrong
      } else if (index != 5 && (note != screen[0] || screen[0] == ' ')) {
        rightInput = 0;
        inputted = 1;
      // No symbol is showing and we received no input
      } else if (screen[0] == ' ' && index == 5 && !inputted) {
        rightInput = 1;
      }

      // Increase speed of game as score rises
      if (currentScore < 250) {
        counter = (counter + 1) % (750 - currentScore * 3);
      } else {
        counter = (counter + 1) % 2;  // Cap on difficulty increase
      }
    }
    write(fd_lcd, "                                                                 ", SCREEN_SIZE * 3);
    highScore = printLose(currentScore, highScore);
    usleep(250000);

    closeBuzzer();

    // Prompts user to choose to play again
    pressAnyButton();
    usleep(500000);
    quit = wantToQuit();
    usleep(500000);
  }
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

// Closes files associated with the buzzer
void closeBuzzer() {
  buzzer(0);
  if (pwm != NULL) fclose(pwm);
  if (dirduty != NULL) fclose(dirduty);
  if (dirT != NULL) fclose(dirT);
}

// Plays given sound on the buzzer
void buzzer(int note) {
  fprintf(dirT, "%d", note);
  fflush(dirT);
  fprintf(dirduty, "%d", note / 2);
  fflush(dirduty);
}

// Opens files used to interface with kernal drivers for LCDs and 5-way button
void openPath(){
  fd_lcd = open(NEW_LCD_DIR, O_RDWR);
  fd_but = open(NEW_BUT_DIR, O_RDWR);
  if (fd_lcd < -1 || fd_but < -1) {
    if (fd_lcd < -1) {
      printf("File %s cannot be opened\n", NEW_LCD_DIR);
      exit(1);
    } else {
      printf("File %s cannot be opened\n", NEW_BUT_DIR);
      exit(1);
    }
  }
}

// Prints the game session ending screen
int printLose(int currentScore, int highScore) {
  int newHighScore, i, lost;
  char winScreen[SCREEN_SIZE * 3];
  if (currentScore > highScore) {
    lost = 0;
    newHighScore = currentScore;
    printf("\nNEW HIGH SCORE!\n");
    strcpy(winScreen, "NEW HIGH SCORE! ");
  } else {
    lost = 1;
    newHighScore = highScore;
    printf("\nSORRY, YOU LOST!\n");
    strcpy(winScreen, "SORRY, YOU LOST!");
  }

  // Builds string for current score
  char scoreString[17];
  sprintf(scoreString, "Score: %d", currentScore);
  for (i = strlen(scoreString); i < SCREEN_SIZE; i++) {
    scoreString[i] = ' ';
  }
  scoreString[SCREEN_SIZE] = '\0';
  strcat(winScreen, scoreString);

  // Builds string for current high score
  char highString[17];
  sprintf(highString, "High Score: %d", highScore);
  for (i = strlen(highString); i < SCREEN_SIZE; i++) {
    highString[i] = ' ';
  }
  highString[SCREEN_SIZE] = '\0';
  strcat(winScreen, highString);
  write(fd_lcd, winScreen, SCREEN_SIZE * 3);
  if (lost) {
    loseMusic();
  } else {
    winMusic();
  }	
  return newHighScore;
}

// Closes all files for exit when termination signal it received
void sigHandler(int signo) {
  if (signo == SIGINT) {
    closeBuzzer();
    if (fd_lcd != 0) close(fd_lcd);
    if (fd_but != 0) close(fd_but);
    exit(0);
  }
}

// Prints the instructions for the user to view on the terminal on game start up
void instructions(){
  printf("\nHello! Welcome to Button Hero!\n\nINSTRUCTIONS: Playing this game requires one user. Press the corresponding\n");
  printf("button when it gets to the far left of the single lined screen. Current score\nand the number of misses are displayed ");
  printf("on the two-lined LCD screen. A miss is\ngiven on a wrong input or when the user misses an input. The user is allowed %d\nmisses", WRONG_GUESSES);
  printf(" until they lose. The current high score is then displayed to the user\nand the user is prompted to play again.\n");

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
  int cursorOnNo = 1;
  while (input[4] != 1) {
    read(fd_but, input, (NUM_BUTTONS * sizeof(int)));
    if ((cursorOnNo == 1) && (input[RIGHT] == 1)) {
      cursorOnNo = 0;
      quitScreen[17] = ' ';
      quitScreen[22] = '>';
      write(fd_lcd, quitScreen, SCREEN_SIZE * 3);
    } else if ((cursorOnNo == 0) && (input[LEFT] == 1)) {
      cursorOnNo = 1;
      quitScreen[17] = '>';
      quitScreen[22] = ' ';
      write(fd_lcd, quitScreen, SCREEN_SIZE * 3);
    }
  }
  return cursorOnNo;
}
