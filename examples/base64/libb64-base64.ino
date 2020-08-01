/*
 * Reference: https://sourceforge.net/p/libb64/git/ci/master/tree/examples/c-example1.c
 */

#include <libb64/cencode.h>
#include <libb64/cdecode.h>

/* arbitrary buffer size */
#define SIZE 100

void setup() {
  Serial.begin(9600);
  Serial.println();
  
  Serial.println("编码：温度=25.20℃, 湿度=44.00%；");
  Serial.print("结果：");
  Serial.println(encode("温度=25.20℃, 湿度=44.00%；"));
  Serial.println("解码：5rWL6K+V5LitKENoaW5lc2Up6IuxKEVuZ2xpc2gp5paH");
  Serial.print("结果：");
  Serial.println(decode("5rWL6K+V5LitKENoaW5lc2Up6IuxKEVuZ2xpc2gp5paH"));
}

void loop() {

}

char* encode(const char* input) {
  /* set up a destination buffer large enough to hold the encoded data */
  char* output = (char*)malloc(SIZE);
  /* keep track of our encoded position */
  char* c = output;
  /* store the number of bytes encoded by a single call */
  int cnt = 0;
  /* we need an encoder state */
  base64_encodestate s;
  
  /*---------- START ENCODING ----------*/
  /* initialise the encoder state */
  base64_init_encodestate(&s);
  /* gather data from the input and send it to the output */
  cnt = base64_encode_block(input, strlen(input), c, &s);
  c += cnt;
  /* since we have encoded the entire input string, we know that 
     there is no more input data; finalise the encoding */
  cnt = base64_encode_blockend(c, &s);
  c += cnt;
  /*---------- STOP ENCODING  ----------*/
  
  /* we want to print the encoded data, so null-terminate it: */
  *c = 0;
  
  return output;
}

char* decode(const char* input) {
  /* set up a destination buffer large enough to hold the encoded data */
  char* output = (char*)malloc(SIZE);
  /* keep track of our decoded position */
  char* c = output;
  /* store the number of bytes decoded by a single call */
  int cnt = 0;
  /* we need a decoder state */
  base64_decodestate s;
  
  /*---------- START DECODING ----------*/
  /* initialise the decoder state */
  base64_init_decodestate(&s);
  /* decode the input data */
  cnt = base64_decode_block(input, strlen(input), c, &s);
  c += cnt;
  /* note: there is no base64_decode_blockend! */
  /*---------- STOP DECODING  ----------*/
  
  /* we want to print the decoded data, so null-terminate it: */
  *c = 0;
  
  return output;
}
