#include "DHT.h"
#include <LiquidCrystal_I2C.h>

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

byte findI2CAddress() {
  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;
  byte i2cAddr = 0;
  
  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
      {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      if (i2cAddr == 0) {
        i2cAddr = i;
      }
      count++;
      delay (1);  // maybe unneeded?
      } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);
  Serial.println (" device(s).");
  return i2cAddr;
}

// 初始化LCD
// LCD1602设备地址，这里的地址是0x3F，也可能是0x20，或者0x27；SCL接D1，SDA接D2。
LiquidCrystal_I2C lcd(findI2CAddress(), D1, D2);

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  lcd.begin(16, 2);
  lcd.backlight();
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
  }
  delay(2000);
}