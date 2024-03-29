# ESP8266连接OneNET MQTTS

在[注册和测试OneNET MQTTS产品](register-onenet-mqtts.md)中，我们知道，连接OneNET MQTT，既可以不使用SSL进行连接，也可以使用SSL连接。下面分别说明这两种连接方式的代码如何实现。

<br/>

### 安装PubSubClient

在Arduino中，选择菜单【工具】->【管理库】，打开“库管理器”。搜索“pubsubclient”，找到PubSubClient包并安装，如下图所示：

![Arduino_pubsubclient_install](images/onenet/Arduino_pubsubclient_install.png)

安装好后，进入下一步。

<br/>

### 连接OneNET MQTT

在[ESP8266生成OneNET MQTTS密码(SHA256)](../../examples/onenet/generate-onenet-mqtts-password-sha256.ino)的基础上，加入基于PubSubClient的实现，代码如下（见[onenet-mqtt-connect.ino](../../examples/onenet/onenet-mqtt-connect.ino)）：

```c++
#include <ESP8266WiFi.h>
#include <time.h>
#include <sstream>

#include <libb64/cencode.h>
#include <libb64/cdecode.h>

#include <Crypto.h>
#include <SHA256.h>
#include <string.h>

#include <PubSubClient.h>

#define HASH_SIZE 32

#ifndef STASSID
#define STASSID "ssid"
#define STAPSK  "password"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

#define base64_alphabet "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define base64_padding '='

#define SIZE 100
#define ONENET_CLIENT_ID "ESP8266_01"     // 设备名称
#define ONENET_USERNAME "321016"          // 平台分配的产品ID
#define ONENET_MQTT_VERSION "2018-10-31"
#define ONENET_MQTT_METHOD "sha256"
#define DEFAULT_EXPIRE_IN_SECONDS 86400   // 24小时
#define ONENET_DEVICE_KEY "5XHGxLb9Rr25t5po+Y3c/mtqiS6ArjsLtzk/sS/D5x8="

#define ONENET_SERVER "183.230.40.96"
#define ONENET_SERVER_PORT 1883

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient client;
PubSubClient mqttclient(client);
char onenet_token[1024];

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  setClock();

  delay(1000);
  Serial.println("");
  time_t now = time(nullptr);
  Serial.print("Current time in seconds: ");
  Serial.println(now);

  time_t expire_time = now + DEFAULT_EXPIRE_IN_SECONDS;
  Serial.print("Expire time in seconds:  ");
  Serial.println(expire_time);
  Serial.println();

  // 要计算哈希值的字符串
  std::stringstream ss;
  ss << "products/" << ONENET_USERNAME << "/devices/" << ONENET_CLIENT_ID;
  char* temp = &*ss.str().begin();
  char resource[strlen(temp)];
  memcpy(resource, temp, strlen(temp) + 1);
  *temp = 0;
  ss.clear();
  ss.str("");
  ss << expire_time << '\n'
     << ONENET_MQTT_METHOD << '\n'
     << resource << '\n'
     << ONENET_MQTT_VERSION;
  Serial.print("String for signature ");
  temp = &*ss.str().begin();
  char stringForSign[strlen(temp)];
  memcpy(stringForSign, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
  Serial.print("(");
  Serial.print(strlen(stringForSign));
  Serial.println("):");
  Serial.println(stringForSign);
  Serial.println();

  // 得到key_string
  char* key_string = decode(ONENET_DEVICE_KEY);
  Serial.println("Decoded key string: ");
  Serial.print("(");
  Serial.print(strlen(key_string));
  Serial.print(") ");
  Serial.println(key_string);
  Serial.println();

  // 得到key_bytes
  byte key_bytes[strlen(key_string)];
  Serial.println("Decoded key bytes: ");
  for(int i = 0; i < strlen(key_string); i++) {
    key_bytes[i] = key_string[i];
    if(key_bytes[i] < 0x10) Serial.print('0');
    Serial.print(key_bytes[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();

  // 得到hmac_sha256
  uint8_t hmac_result_bytes[HASH_SIZE];
  crypto_hmac_sha256(key_string, strlen(key_string), stringForSign, strlen(stringForSign), hmac_result_bytes);
  Serial.println("HMac-SHA256 bytes:");
  for(int i = 0; i < HASH_SIZE; i++) {
    uint8_t b = hmac_result_bytes[i];
    if(b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();

  char encoded_hmac_result[SIZE];
  base64_string_from_bytes(hmac_result_bytes, sizeof(hmac_result_bytes), encoded_hmac_result);
  Serial.println("Encoded HMac-SHA256 string: ");
  Serial.println(encoded_hmac_result);
  Serial.println();
  Serial.println();

  // 生成password
  ss.clear();
  ss.str("");
  char resource_url_encoded[SIZE];
  url_encode(resource, strlen(resource), resource_url_encoded);
  char hmac_url_encoded[SIZE];
  url_encode(encoded_hmac_result, strlen(encoded_hmac_result), hmac_url_encoded);
  ss << "version=" << ONENET_MQTT_VERSION
     << "&res=" << resource_url_encoded
     << "&et=" << expire_time
     << "&method=" << ONENET_MQTT_METHOD
     << "&sign=" << hmac_url_encoded;
  Serial.println("OneNET password: ");
  temp = &*ss.str().begin();
  onenet_token[strlen(temp)];
  memcpy(onenet_token, temp, strlen(temp) + 1);  
  Serial.println(onenet_token);
  ss.clear();
  ss.str("");

  // 配置mqtt客户端参数
  mqttclient.setServer(ONENET_SERVER, ONENET_SERVER_PORT);
  mqttclient.setKeepAlive(60); // 单位是秒
  mqttclient.setSocketTimeout(30); // 单位是秒
  mqttclient.setCallback(callback);
}

void loop() {
  if (!mqttclient.loop()) {
    Serial.println("Connecting to public OneNET mqtt broker.....");
    if (mqttclient.connect(ONENET_CLIENT_ID, ONENET_USERNAME, onenet_token)) {
      Serial.println("OneNET mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(mqttclient.state());
    }
  } else {
    Serial.println("Connection to public OneNET mqtt broker is alive!");
  }
  delay(10000);
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

void crypto_hmac_sha256(char* key, size_t keyLength, char* msg, size_t msgLength, uint8_t* result) {
  uint8_t value[HASH_SIZE];
  SHA256 sha256Hash;
  sha256Hash.resetHMAC(key, keyLength);
  sha256Hash.update(msg, msgLength);
  sha256Hash.finalizeHMAC(key, keyLength, value, HASH_SIZE);
  memcpy(result, value, HASH_SIZE);
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void base64_string_from_bytes(byte bytes[], int bytes_size, char result_char[]) {
  byte byte1, byte2, byte3;
  char octet1, octet2, octet3, octet4;
  std::stringstream ss;

  for (int i = 0; i < bytes_size; i += 3) {
    // Collect pair bytes
    byte1 = bytes[i];
    byte2 = i + 1 < bytes_size ? bytes[i + 1] : NULL;
    byte3 = i + 2 < bytes_size ? bytes[i + 2] : NULL;

    // Bits 1-6 from byte 1
    octet1 = byte1 >> 2;

    // Bits 7-8 from byte 1 joined by bits 1-4 from byte 2
    octet2 = ((byte1 & 3) << 4) | (byte2 >> 4);

    // Bits 4-8 from byte 2 joined by bits 1-2 from byte 3
    octet3 = ((byte2 & 15) << 2) | (byte3 >> 6);

    // Bits 3-8 from byte 3
    octet4 = byte3 & 63;

    // Map octets to characters
    ss << base64_alphabet[octet1] 
       << base64_alphabet[octet2]
       << (byte2 != NULL ? base64_alphabet[octet3] : base64_padding)
       << (byte3 != NULL ? base64_alphabet[octet4] : base64_padding);
  }
  char* temp = &*ss.str().begin();
  memcpy(result_char, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
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
```

其中，与OneNET MQTT相关的代码是：

```c++
...
#include <PubSubClient.h>
...
#define ONENET_SERVER "183.230.40.96"
#define ONENET_SERVER_PORT 1883

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient client;
PubSubClient mqttclient(client);
...
setup() {
  // 配置mqtt客户端参数
  mqttclient.setServer(ONENET_SERVER, ONENET_SERVER_PORT);
  mqttclient.setKeepAlive(60); // 单位是秒
  mqttclient.setSocketTimeout(30); // 单位是秒
  mqttclient.setCallback(callback);
}

void loop() {
  if (!mqttclient.loop()) {
    Serial.println("Connecting to public OneNET mqtt broker.....");
    if (mqttclient.connect(ONENET_CLIENT_ID, ONENET_USERNAME, onenet_token)) {
      Serial.println("OneNET mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(mqttclient.state());
    }
  } else {
    Serial.println("Connection to public OneNET mqtt broker is alive!");
  }
  Serial.println();
  delay(10000);
}
```

执行代码，在串口监视器中的输出如下图所示：

![esp8266-onenet-mqtt-connect.png](images/onenet/esp8266-onenet-mqtt-connect.png)

<br/>

### 连接OneNET MQTTS

继续修改上述代码，加入使用SSL，代码如下（见[onenet-mqtts-connect.ino](../../examples/onenet/onenet-mqtts-connect.ino)）：

```c++
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <sstream>

#include <libb64/cencode.h>
#include <libb64/cdecode.h>

#include <Crypto.h>
#include <SHA256.h>
#include <string.h>

#include <PubSubClient.h>

#define HASH_SIZE 32

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-passwd"
#endif

const char *ssid = STASSID;
const char *pass = STAPSK;

#define base64_alphabet "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
#define base64_padding '='

#define SIZE 100
#define ONENET_CLIENT_ID "ESP8266_01"     // 设备名称
#define ONENET_USERNAME "321016"          // 平台分配的产品ID
#define ONENET_MQTT_VERSION "2018-10-31"
#define ONENET_MQTT_METHOD "sha256"
#define DEFAULT_EXPIRE_IN_SECONDS 86400   // 24小时
#define ONENET_DEVICE_KEY "5XHGxLb9Rr25t5po+Y3c/mtqiS6ArjsLtzk/sS/D5x8="

IPAddress mqtt_ip(183, 230, 40, 16);
#define ONENET_SERVER "183.230.40.16"
#define ONENET_SERVER_PORT 8883

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    char receivedChar = (char)payload[i];
    Serial.println(receivedChar);
    //****
    //Do some action based on message received
    //***
  }
}

static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV
BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X
DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM
BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx
5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR
r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS
UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl
U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP
ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G
A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia
bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL
aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7
q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb
41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a
3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR
NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa
WzdhzTYwVkxBaU+xf/2w
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure client; 
PubSubClient mqttclient(client);
X509List cert(cacert);
char onenet_token[1024];

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  setClock();

  delay(1000);
  Serial.println("");
  time_t now = time(nullptr);
  Serial.print("Current time in seconds: ");
  Serial.println(now);

  time_t expire_time = now + DEFAULT_EXPIRE_IN_SECONDS;
  Serial.print("Expire time in seconds:  ");
  Serial.println(expire_time);
  Serial.println();

  // 要计算哈希值的字符串
  std::stringstream ss;
  ss << "products/" << ONENET_USERNAME << "/devices/" << ONENET_CLIENT_ID;
  char* temp = &*ss.str().begin();
  char resource[strlen(temp)];
  memcpy(resource, temp, strlen(temp) + 1);
  *temp = 0;
  ss.clear();
  ss.str("");
  ss << expire_time << '\n'
     << ONENET_MQTT_METHOD << '\n'
     << resource << '\n'
     << ONENET_MQTT_VERSION;
  Serial.print("String for signature (");
  temp = &*ss.str().begin();
  char stringForSign[strlen(temp)];
  memcpy(stringForSign, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
  Serial.print(strlen(stringForSign));
  Serial.println("):");
  Serial.println(stringForSign);
  Serial.println();

  // 得到key_string
  char* key_string = decode(ONENET_DEVICE_KEY);
  Serial.print("Decoded key string(");
  Serial.print(strlen(key_string));
  Serial.println("):");
  Serial.println(key_string);
  Serial.println();

  // 得到key_bytes
  byte key_bytes[strlen(key_string)];
  Serial.println("Decoded key bytes: ");
  for(int i = 0; i < strlen(key_string); i++) {
    key_bytes[i] = key_string[i];
    if(key_bytes[i] < 0x10) Serial.print('0');
    Serial.print(key_bytes[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();

  // 得到hmac_sha256
  uint8_t hmac_result_bytes[HASH_SIZE];
  crypto_hmac_sha256(key_string, strlen(key_string), stringForSign, strlen(stringForSign), hmac_result_bytes);
  Serial.println("HMac-SHA256 bytes:");
  for(int i = 0; i < HASH_SIZE; i++) {
    uint8_t b = hmac_result_bytes[i];
    if(b < 0x10) Serial.print('0');
    Serial.print(b, HEX);
    Serial.print(' ');
  }
  Serial.println();
  Serial.println();

  char encoded_hmac_result[SIZE];
  base64_string_from_bytes(hmac_result_bytes, sizeof(hmac_result_bytes), encoded_hmac_result);
  Serial.println("Encoded HMac-SHA256 string: ");
  Serial.println(encoded_hmac_result);
  Serial.println();
  Serial.println();

  // 生成password
  ss.clear();
  ss.str("");
  char resource_url_encoded[SIZE];
  url_encode(resource, strlen(resource), resource_url_encoded);
  char hmac_url_encoded[SIZE];
  url_encode(encoded_hmac_result, strlen(encoded_hmac_result), hmac_url_encoded);
  ss << "version=" << ONENET_MQTT_VERSION
     << "&res=" << resource_url_encoded
     << "&et=" << expire_time
     << "&method=" << ONENET_MQTT_METHOD
     << "&sign=" << hmac_url_encoded;
  Serial.println("OneNET password: ");
  temp = &*ss.str().begin();
  onenet_token[strlen(temp)];
  memcpy(onenet_token, temp, strlen(temp) + 1);
  Serial.println(onenet_token);
  ss.clear();
  ss.str("");

  // client.setInsecure();
  client.setTrustAnchors(&cert);
  setClock();

  // 配置MQTT客户端
  // mqttclient.setServer(ONENET_SERVER, ONENET_SERVER_PORT);
  mqttclient.setServer(mqtt_ip, ONENET_SERVER_PORT);
  mqttclient.setKeepAlive(60);
  mqttclient.setSocketTimeout(30);
  mqttclient.setCallback(callback);
}

void loop() {
  if (!mqttclient.loop()) {
    Serial.println("Connecting to public OneNET mqtts broker.....");
    if (mqttclient.connect(ONENET_CLIENT_ID, ONENET_USERNAME, onenet_token)) {
      Serial.println("OneNET mqtts broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(mqttclient.state());
    }
  } else {
    Serial.println("Connection to public OneNET mqtts broker is alive!");
  }
  Serial.println();
  delay(10000);
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

void crypto_hmac_sha256(char* key, size_t keyLength, char* msg, size_t msgLength, uint8_t* result) {
  uint8_t value[HASH_SIZE];
  SHA256 sha256Hash;
  sha256Hash.resetHMAC(key, keyLength);
  sha256Hash.update(msg, msgLength);
  sha256Hash.finalizeHMAC(key, keyLength, value, HASH_SIZE);
  memcpy(result, value, HASH_SIZE);
}

// Set time via NTP, as required for x.509 validation
void setClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void base64_string_from_bytes(byte bytes[], int bytes_size, char result_char[]) {
  byte byte1, byte2, byte3;
  char octet1, octet2, octet3, octet4;
  std::stringstream ss;

  for (int i = 0; i < bytes_size; i += 3) {
    // Collect pair bytes
    byte1 = bytes[i];
    byte2 = i + 1 < bytes_size ? bytes[i + 1] : NULL;
    byte3 = i + 2 < bytes_size ? bytes[i + 2] : NULL;

    // Bits 1-6 from byte 1
    octet1 = byte1 >> 2;

    // Bits 7-8 from byte 1 joined by bits 1-4 from byte 2
    octet2 = ((byte1 & 3) << 4) | (byte2 >> 4);

    // Bits 4-8 from byte 2 joined by bits 1-2 from byte 3
    octet3 = ((byte2 & 15) << 2) | (byte3 >> 6);

    // Bits 3-8 from byte 3
    octet4 = byte3 & 63;

    // Map octets to characters
    ss << base64_alphabet[octet1] 
       << base64_alphabet[octet2]
       << (byte2 != NULL ? base64_alphabet[octet3] : base64_padding)
       << (byte3 != NULL ? base64_alphabet[octet4] : base64_padding);
  }
  char* temp = &*ss.str().begin();
  memcpy(result_char, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
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
```

其中，使用SSL CA证书进行认证的代码，参考[ESP8266访问Https网页](esp8266-wifi-ssl.md)，这里的代码如下：

```c++
static const char cacert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDOzCCAiOgAwIBAgIJAPCCNfxANtVEMA0GCSqGSIb3DQEBCwUAMDQxCzAJBgNV
BAYTAkNOMQ4wDAYDVQQKDAVDTUlPVDEVMBMGA1UEAwwMT25lTkVUIE1RVFRTMB4X
DTE5MDUyOTAxMDkyOFoXDTQ5MDUyMTAxMDkyOFowNDELMAkGA1UEBhMCQ04xDjAM
BgNVBAoMBUNNSU9UMRUwEwYDVQQDDAxPbmVORVQgTVFUVFMwggEiMA0GCSqGSIb3
DQEBAQUAA4IBDwAwggEKAoIBAQC/VvJ6lGWfy9PKdXKBdzY83OERB35AJhu+9jkx
5d4SOtZScTe93Xw9TSVRKrFwu5muGgPusyAlbQnFlZoTJBZY/745MG6aeli6plpR
r93G6qVN5VLoXAkvqKslLZlj6wXy70/e0GC0oMFzqSP0AY74icANk8dUFB2Q8usS
UseRafNBcYfqACzF/Wa+Fu/upBGwtl7wDLYZdCm3KNjZZZstvVB5DWGnqNX9HkTl
U9NBMS/7yph3XYU3mJqUZxryb8pHLVHazarNRppx1aoNroi+5/t3Fx/gEa6a5PoP
ouH35DbykmzvVE67GUGpAfZZtEFE1e0E/6IB84PE00llvy3pAgMBAAGjUDBOMB0G
A1UdDgQWBBTTi/q1F2iabqlS7yEoX1rbOsz5GDAfBgNVHSMEGDAWgBTTi/q1F2ia
bqlS7yEoX1rbOsz5GDAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAL
aqJ2FgcKLBBHJ8VeNSuGV2cxVYH1JIaHnzL6SlE5q7MYVg+Ofbs2PRlTiWGMazC7
q5RKVj9zj0z/8i3ScWrWXFmyp85ZHfuo/DeK6HcbEXJEOfPDvyMPuhVBTzuBIRJb
41M27NdIVCdxP6562n6Vp0gbE8kN10q+ksw8YBoLFP0D1da7D5WnSV+nwEIP+F4a
3ZX80bNt6tRj9XY0gM68mI60WXrF/qYL+NUz+D3Lw9bgDSXxpSN8JGYBR85BxBvR
NNAhsJJ3yoAvbPUQ4m8J/CoVKKgcWymS1pvEHmF47pgzbbjm5bdthlIx+swdiGFa
WzdhzTYwVkxBaU+xf/2w
-----END CERTIFICATE-----
)EOF";
...
X509List cert(cacert);
...
  client.setTrustAnchors(&cert);
```

上面的CA证书，是从[OneNET MQTT物联网套件开发指南](https://open.iot.10086.cn/doc/mqtt/book/device-develop/manual.html)页面上下载的CA证书。当然，不检查证书，或者说信任任何证书，也是可以的。例如，直接使用：

```c++
  client.setInsecure();
```

**在设置OneNET MQTTS服务器地址时，必须使用IPAddress**:

```C++
IPAddress mqtt_ip(183, 230, 40, 16);
...
  mqttclient.setServer(mqtt_ip, ONENET_SERVER_PORT);
```

执行代码，在串口监视器中的输出如下图所示：

![esp8266-onenet-mqtts-connect.png](images/onenet/esp8266-onenet-mqtts-connect.png)

<br/>

**如果使用域名方式连接OneNET MQTTS服务器，会失败**：

```c++
#define ONENET_SERVER "183.230.40.16"
...
  mqttclient.setServer(ONENET_SERVER, ONENET_SERVER_PORT);
```

再次执行代码，连接OneNET MQTTS服务器失败，在串口监视器中的输出如下图所示：

![esp8266-onenet-mqtts-connect-failed.png](images/onenet/esp8266-onenet-mqtts-connect-failed.png)


<br/>

### 参考资料

1. Arduino PubSubClient: [https://www.arduinolibraries.info/libraries/pub-sub-client](https://www.arduinolibraries.info/libraries/pub-sub-client)

2. Auduino PubSubClient源代码：[https://github.com/knolleary/pubsubclient/](https://github.com/knolleary/pubsubclient/)

3. OneNET MQTT物联网套件开发指南：[https://open.iot.10086.cn/doc/mqtt/book/device-develop/manual.html](https://open.iot.10086.cn/doc/mqtt/book/device-develop/manual.html)
