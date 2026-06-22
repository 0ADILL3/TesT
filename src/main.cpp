#include <Arduino.h> // Wajib ditambahkan di PlatformIO (berbeda dengan Arduino IDE)

// Pin LED bawaan pada sebagian besar ESP32 DevKit adalah pin 2
#define LED_PIN 2

void setup() {
  // Memulai komunikasi serial dengan kecepatan 115200 bps
  Serial.begin(115200);
  
  // Mengatur pin LED sebagai jalur keluar (Output)
  pinMode(LED_PIN, OUTPUT);
  
  // Memberi jeda sebentar agar Serial Monitor siap
  delay(1000); 
  Serial.println("\n--- ESP32 Berhasil Menyala! ---");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);  // Menyalakan LED
  Serial.println("Status: LED Nyala");
  delay(1000);                  // Menunggu 1 detik (1000 milidetik)
  
  digitalWrite(LED_PIN, LOW);   // Mematikan LED
  Serial.println("Status: LED Mati");
  delay(1000);                  // Menunggu 1 detik
}