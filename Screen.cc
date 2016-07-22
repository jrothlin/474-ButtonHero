/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * Screen.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program controls the functionality of the entire screen used to play
 * the game.
 */

Screen::Screen() {
  game_screen_ = new GameScreen();
  score_board_ = new ScoreBoard();
}

void Screen::displayWaitingScreen() {
  gameScreen_.blank();
  scoreBoard_.promptForInput();
}

void Screen::displayGameOver(bool playerWon, int gameScore, int highScore) {
  if (playerWon) {
    game_screen_.displayWin();
  } else {
    game_screen_.displayLose();
  }
  score_board_.displayScores(gameScore, highScore);
}

void Screen::displayReplayPrompt(bool selection) {
  game_screen_.blank();
  score_board_.displayReplayAndChoices(selection);
}

void Screen::shutDown() {
  gameScreen_.shutDown();
  score_board_.shutDown();
}
