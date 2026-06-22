#include <Arduino.h>

#define LED_PIN 2

void setup()
{
  Serial.begin(115200);
  
  pinMode(LED_PIN, OUTPUT);
  
  delay(1000); 
  Serial.println("\n--- ESP32 Berhasil Menyala! ---");
}

void loop()
{
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Status: LED Nyala");
  delay(1000);
  
  digitalWrite(LED_PIN, LOW);
  Serial.println("Status: LED Mati");
  delay(1000);
}