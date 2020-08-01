/**
* Download from https://github.com/Densaugeo/base64_arduino and unzip it to
* libraries folder.
*/

#include "base64.hpp"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  unsigned char binary[] = {133, 244, 117, 206, 178, 195};
  unsigned char base64[9];

  unsigned int base64_length = encode_base64(binary, 6, base64);

  Serial.println(base64_length); // Prints "8"
  Serial.println((char *) base64); // Prints "hfR1zrLD"

  unsigned char newBase64[] = "hfR1zrLD";
  unsigned int binary_length = decode_base64(newBase64, binary);

  Serial.print(binary[0], HEX);
  Serial.print(binary[1], HEX);
  Serial.print(binary[2], HEX);
  Serial.print(binary[3], HEX);
  Serial.print(binary[4], HEX);
  Serial.println(binary[5], HEX);
  Serial.println(binary_length); // Prints "6"
}

void loop() {
  // put your main code here, to run repeatedly:

}
