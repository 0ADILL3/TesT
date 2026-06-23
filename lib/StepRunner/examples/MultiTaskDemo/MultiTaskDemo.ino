#include <StepRunner.h>

#define LED1 13
#define LED2 12

void blinkLed1() {
  digitalWrite(LED1, !digitalRead(LED1));
}

void blinkLed2() {
  digitalWrite(LED2, !digitalRead(LED2));
}

void printSerial() {
  Serial.println("Running periodic task...");
}

StepRunner led1Task(blinkLed1, 500);
StepRunner led2Task(blinkLed2, 300);
StepRunner printTask(printSerial, 2000);

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  led1Task.run();   // LED1 blink tiap 500ms
  led2Task.run();   // LED2 blink tiap 300ms
  printTask.run();  // Serial print tiap 2 detik
}