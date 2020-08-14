#include <sstream>

#define STRING_TO_ENCODE "D7Orwse/i7lkRHQjQ8YQ2g=="
#define SIZE 100

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("String to url encode: ");
  Serial.println(STRING_TO_ENCODE);
  char string_encoded[SIZE];
  url_encode(STRING_TO_ENCODE, strlen(STRING_TO_ENCODE), string_encoded);
  Serial.println("String encoded: ");
  Serial.println(string_encoded);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void url_encode(char str_to_encode[], int str_length, char result_char[]) {
  std::stringstream ss;
  for (int i = 0; i < str_length; i++){
    char c = str_to_encode[i];
    if (c == '+'){
      ss << "%2B";
    } else if (c == ' '){
      ss << "%20";
    } else if (c == '/') {
      ss << "%2F";
    } else if (c == '?') {
      ss << "%3F";
    } else if (c == '%') {
      ss << "%25";
    } else if (c == '#') {
      ss << "%23";
    } else if (c == '&') {
      ss << "%26";
    } else if (c == '=') {
      ss << "%3D";
    } else {
      ss << c;
    }
  }
  char* temp = &*ss.str().begin();
  memcpy(result_char, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}