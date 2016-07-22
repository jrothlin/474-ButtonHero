/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * ButtonHero.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program plays a game synonymous to "Guitar Hero" entitled "Button Hero"
 * Symbols scroll across a screen as the user attempt to press the
 * corresponding buttons at the correct time. This program is targeted for
 * the ARM-Cortex processor aboard the Beaglebone Black and uses two kernal
 * module drivers (lcd_driver and button_driver) to interface with the hardware
 * used for this game.
 */

int main(int argc, char** argv) {
  Game mainGame = new Game();
  while (!mainGame.quit_) {
    mainGame.initialWaitingScreen();
    mainGame.beginPlaying();
    mainGame.playAgainPrompt();
  }
  mainGame.shutDown();
  return EXIT_SUCCESS;
}
