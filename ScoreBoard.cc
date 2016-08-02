/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * ScoreBoard.cc
 *
 * Copyright 2016 Joseph Rohtlin
 *
 * This program represents the screen on which the score of the game is
 * diplayed.
 */

#define SCREEN_SIZE 32

ScoreBoard::ScoreBoard() {
  score_board_characters_ = stringOfSpaces(SCREEN_SIZE);
}

void ScoreBoard::setToWaitingScreen() {
  score_board_charactes_ = "Press any button to continue";
}

void ScoreBoard::displayScores(int score, int highScore) {
  score_board_characters = "High Score: " + score + "Your score: " + highScore;
}

void ScoreBoard::displayReplayPromptSelectYes() {
  score_board_characters_ = "Would you like to play again? >Yes  No";
}

void ScoreBoard::displayReplayPromptSelectNo() {
  score_board_characters_ = "Would you like to play again?  Yes >No";
}

void ScoreBoard::shutDown() {

}

private String stringOfSpaces(int length) {
  String emptyString = "";
  for (int i = 0; i < length; i++) {
    emptyString += " ";
  }
  return emptyString;
}
