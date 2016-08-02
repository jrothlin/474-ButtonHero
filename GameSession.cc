/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * GameSession.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program implements one game session which is defined as one beginning
 * when the symbols begin scrolling across the screen and ending when the
 * player makes too many mistaces.
 */

#define MISSES_ALOUD 5

GameSession::GameSession(Player player, Screen screen) : player_(player), screen_(screen) {}

int GameSession::play() {
  // initialize the screens
  // turn on button sounds for the player
  current_misses_ = 0;
  while (current_misses <= MISSES_ALOUD) {
    // screen updates that happens every n loops
    // very short delay
    // read the button status
    // update the session state
  }
  // turn off button sounds for the player
}
