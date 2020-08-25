#include "DHT.h"
#include <LiquidCrystal.h>

// for Arduino
// #define DHTPIN 8

// for ESP8266
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define RX 3 // D9
#define TX 1 // D10

#define DHTPIN RX
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

// LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(D0, D1, D2, D3, D4, D5);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp=");
    lcd.print(t);
    lcd.write((byte)223);
    lcd.print("C");
    lcd.setCursor(0,1);
    lcd.print("Humidity=");
    lcd.print(h);
    lcd.print("% ");
    Serial.print("Temp=");
    Serial.print(t);
    Serial.println("℃");
    delay(2000);
  }
}