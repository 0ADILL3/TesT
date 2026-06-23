#ifndef STEPRUNNER_H
#define STEPRUNNER_H

#include <Arduino.h>

class StepRunner {
  private:
    unsigned long _lastTime;
    unsigned long _interval;
    void (*_process)();

  public:
    StepRunner(void (*process)(), unsigned long interval);
    void run();
    void setInterval(unsigned long interval);
    void resetTimer();
};

#endif