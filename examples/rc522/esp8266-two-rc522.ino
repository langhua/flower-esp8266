/**
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read data from more than one PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 *
 * Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
 * MFRC522 based RFID Reader on the Arduino SPI interface.
 *
 * Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
 *          and knowledge are required!
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required **
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */

#include <SPI.h>
#include <MFRC522.h>

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

// for ESP8266, SDA_1(SS_1) -> D8, SCK -> D5, MOSI -> D7, MISO -> D6, RST -> D3
//              SDA_2(SS_2) -> D4
#define RST_PIN         D3          // Configurable, see typical pin layout above
#define SS_1_PIN        D8          // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define SS_2_PIN        D4          // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1

#define NR_OF_READERS   2

byte ssPins[] = {SS_1_PIN, SS_2_PIN};

MFRC522 mfrc522[NR_OF_READERS];   // Create MFRC522 instance.

/**
 * Initialize.
 */
void setup() {

  Serial.begin(115200); // Initialize serial communications with the PC
  while (!Serial);

  SPI.begin();        // Init SPI bus

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    mfrc522[reader].PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_48dB); // 0x07 << 4
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }

  delay(1000);
}

uint8_t counter = 0;

/**
 * Main loop.
 */
void loop() {

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    // Look for new cards
    uint8_t i = 1;
    while (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F(": Card["));
      Serial.print(i);
      Serial.print(F("] UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
      // delay(100);
      i++;
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader

  counter++;
  if (counter >= 10) {
    Serial.println();
    // reinitialize reader1
    mfrc522[1].PCD_Init(ssPins[1], RST_PIN);
    mfrc522[1].PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_48dB);
    Serial.print(F("Reader 1: "));
    mfrc522[1].PCD_DumpVersionToSerial();
    counter = 0;
  } else {
    Serial.print(".");
  }
  delay(1000);
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
