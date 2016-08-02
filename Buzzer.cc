/*
 * The entirety of this project can be found at:
 *   https://github.com/jrothlin
 *
 * Buzzer.cc
 *
 * Copyright 2016 Joseph Rothlin
 *
 * This program controls the functionality of the buzzer.
 */

#define PWM_INITIALIZATION_FILE_DIR "/sys/devices/bone_capemgr.9/slots"
#define PERIOD_FILE_DIR "/sys/devices/ocp.3/pwm_test_P9_14.15/duty"
#define DUTY_FILE_DIR "/sys/devices/ocp.3/pwm_test_P9_14.15/period"

Buzzer::Buzzer() {
  pwm_initialization_dir_ = fopen(PWM_INITIALIZATION_FILE_DIR, "w");
  fseek(pwm_initialization_dir_, 0, SEEK_END);
  fprintf(pwm_initialization_dir_, "am33xx_pwm");
  fprintf(pwm_initialization_dir_, "bone_pwm_P9_14");
  period_dir_ = fopen(PERIOD_FILE_DIR, "w");
  duty_dir_ = fopen(DUTY_FILE_DIR, "w");
}

void Buzzer::setVolume(const int volume) {

}

void Buzzer::playNote(const int note) {

}

void Buzzer::shutDown() {
  fclose(pwm_initialization_dir_);
  fclose(period_dir_);
  fclose(duty_dir_);
}
