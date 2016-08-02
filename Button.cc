/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * Button.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program controls the functionality of the control button.
 */

#define BUTTON_DIR "/dev/button_driver"
#define NUM_BUTTONS 5

Button::Button() {
  button_fd_ = open(BUTTON_DIR, O_RDWR);
}

int Button::getStatus() {
  read(button_fd_, inputStatus, NUM_BUTTONS * sizeof(int));
  for (int i = 0; i < NUM_BUTTONS; i++)
    if (inputStatus[i] == 1) return i + 1;
  return 0;
}

void Button::shutDown() {
  close(button_fd_);
}
