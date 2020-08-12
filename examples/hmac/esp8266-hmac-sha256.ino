/*
  Crypto SHA256/HMAC-SHA256 implementation to verify correct behaviour.
*/

#include <Crypto.h>
#include <SHA256.h>
#include <string.h>

#define HASH_SIZE 32

char* message = "ver=1.1&temp=22.4℃&temp_text=温度&humidity=48.0%&humidity_text=湿度";
char* key = "yourKey";

void setup() {
  Serial.begin(9600);
  Serial.println();
  
  Serial.println("测试SHA256");
  Serial.println("--------------------------------------------");
  Serial.print("字符串：");
  Serial.println(message);
  
  uint8_t result[HASH_SIZE];
  crypto_sha256(message, strlen(message), result);
  
  Serial.print("SHA256结果: ");
  for(int i = 0; i < HASH_SIZE; i++) {
    uint8_t b = result[i];
    if(b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }

  Serial.println();
  Serial.println();
  Serial.println("测试HMAC-SHA256");
  Serial.println("--------------------------------------------");
  Serial.print("字符串：");
  Serial.println(message);
  Serial.print("HMAC KEY: ");
  for(int i = 0; i < strlen(key); i++) {
    uint8_t b = key[i];
    if(b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();

  crypto_hmac_sha256(key, strlen(key), message, strlen(message), result);

  Serial.print("HMAC-SHA256结果: ");
  for(int i = 0; i < HASH_SIZE; i++) {
    byte b = result[i];
    if(b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
}

void loop() {

}

void crypto_sha256(char* msg, size_t msgLength, uint8_t* result) {
  uint8_t value[HASH_SIZE];
  SHA256 sha256Hash;
  sha256Hash.reset();
  sha256Hash.update(msg, msgLength);
  sha256Hash.finalize(value, HASH_SIZE);
  memcpy(result, value, HASH_SIZE);
}

void crypto_hmac_sha256(char* key, size_t keyLength, char* msg, size_t msgLength, uint8_t* result) {
  uint8_t value[HASH_SIZE];
  SHA256 sha256Hash;
  sha256Hash.resetHMAC(key, keyLength);
  sha256Hash.update(msg, msgLength);
  sha256Hash.finalizeHMAC(key, keyLength, value, HASH_SIZE);
  memcpy(result, value, HASH_SIZE);
}
