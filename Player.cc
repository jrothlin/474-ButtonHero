/*
 * The entirety of this project can be found at:
 *   https://gitbuh.com/jrothlin
 *
 * Player.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program controls the functionality associated with the player of the
 * game.
 */

#define NONE 0;
#define UP 1;
#define DOWN 2;
#define LEFT 3;
#define RIGHT 4;
#define SELECT 5;

Player::Player() {
  buzzer_ = new Buzzer();
  button_ = new Button();
}

void Player::getInput() {
  int input = 0;
  while (input == 0) {
    button_.getStatus();
  }
  return input;
}

void Player::shutDown() {
  buzzer_.shutDown();
  button_.shutDown();
}
