/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * GameScreen.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This is a singleton object which represents the screen on which the
 * characters scroll across the screen. 
 */

#define SCREEN_SIZE 32

GameScreen::GameScreen() {
  game_screen_characters_ = stringOfSpcaes(SCREEN_SIZE);
}

void GameScreen::blank() {
  game_screen_characters_ = stringOfSpaces(SCREEN_SIZE);
}

void GameScreen::displayGameOver(bool win) {
  if (win) {
    game_screen_chracters_ = "CONGRATULATIONS! NEW HIGH SCORE!";
  } else {
    game_screen_chracters_ = "SORRY! YOU LOSE!";
  }
}

void GameScreen::shutDown() {

}

private String stringOfSpaces(int length) {
  String emptyString = "";
  for (int i = 0; i < length; i++) {
    emptyString += " ";
  }
  return emptyString;
}
