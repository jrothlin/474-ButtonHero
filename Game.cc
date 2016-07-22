/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * Game.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program encapsulates the functionality of an instance of the game.
 */

#define NONE 0;
#define UP 1;
#define DOWN 2;
#define LEFT 3;
#define RIGHT 4;
#define SELECT 5;

#define SELECT_YES true;
#define SELECT_NO false;

#define WIN true;
#define LOSE false;

Game::Game() {
  connection_ = new Connection();
  screen_ = new Screen();
  player_ = new Player();
  quit_ = false;
  high_score_ = 0;
}

void Game::initialWaitingScreen() const {
  screen_.displayWaitingScreen();
  player_.getInput();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Game::beginPlaying() {
  GameSession gameSession = new GameSession(screen_, player_);
  int score = gameSession.play();
  bool gameResult;
  if (score >= high_score_) {
    high_score_ = score;
    gameResult = WIN;
  } else {
    gameResult = LOSE;
  }
  screen_.displayGameOver(gameResult, score, high_score_);
  player_.getInput();
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

void Game::playAgainPrompt() {
  int buttonPressed = -1;
  bool selectionStatus = SELECT_NO;
  screen_.displayReplayPrompt(SELECT_NO);
  while (buttonPressed != SELECT) {
    buttonPressed = player_.getInput();
    if (buttonPressed == 3 && selectionStatus == SELECT_YES) {
      selectionStatus = SELECT_NO;
    } else if (buttonPressed == 4 && selectionStatus == SELECT_NO) {
      selectionStatus = SELECT_YES;
    }
  }
  if (SELECT_NO) quit_ = true;
}

void Game::shutDown() {
  screen_.shutDown();
  player_.shutDown();
}

