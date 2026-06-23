#include "StepRunner.h"

StepRunner::StepRunner(void (*process)(), unsigned long interval) {
  _process = process;
  _interval = interval;
  _lastTime = millis();
}

void StepRunner::run() {
  if (millis() - _lastTime >= _interval) {
    _process();
    _lastTime = millis();
  }
}

void StepRunner::setInterval(unsigned long interval) {
  _interval = interval;
}

void StepRunner::resetTimer() {
  _lastTime = millis();
}