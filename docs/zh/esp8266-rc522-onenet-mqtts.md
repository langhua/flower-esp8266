# ESP8266 RC522读卡

本节在[ESP8266接受OneNET MQTTS命令](esp8266-onenet-mqtts-command.md)的基础上，增加了ESP8266使用RC522读取任意RFID卡，向SG90发送一条转动到0度或90度命令。

### Fritzing元件图
* [RC522-Fritzing元件图](https://github.com/miguelbalboa/rfid/blob/master/doc/fritzing/)，导入其中的RFID-RC522-v3.fzpz文件

接线图如下图所示：

![NodeMCU-DHT11-LCD1602_I2C-SG90-RC522](images/rc522/ESP8266-DHT11-LCD1602_I2C-SG90-RC522.png)

<br/>

### 基于MFRC522 by GithubCommunity的实现

在Arduino IDE的【库管理】中，安装MFRC522，如下图所示：
![Arduino_rc522_searchresults](images/rc522/arduino-mfrc522-lib.png)

<br/>

### MFRC522样例代码

在Arduino中，修改和运行MFRC522库带的ReadNUID样例程序。调出ReadNUID示例代码的方法是，点击菜单【文件】->【示例】->【MFRC522】->【ReadNUID】，如下图所示：

![Arduino-MFRC522-ReadNUID](images/rc522/arduino-mfrc522-ReadNUID.png)

ReadNUID样例程序，要修改为如下所示代码，才能符合前面的线路图接线，正确执行：

```c++
/**
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
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
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
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

// for ESP8266, SDA(SS) -> D8, SCK -> D5, MOSI -> D7, MISO -> D6, RST -> D3
#define SS_PIN D8
#define RST_PIN D3
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
 
void loop() {

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
```

第34-49行，是针对ESP8266做的修改，替代了原来ReadNUID的第34和35行。

打开串口监视器，设为9600波特率，执行上述代码，同时使用两张RFID卡片，如果能显示出两张卡NUID，即RF522和接线均正确可用。

<br/>

### 代码说明

下面的代码是在[ESP8266接受OneNET MQTTS命令](esp8266-onenet-mqtts-command.md)基础上，结合本文上面的RC522读卡代码修改而成：

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

#include "DHT.h"
#include <LiquidCrystal_I2C.h>

#include <Servo.h>

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

// for ESP8266, SDA(SS) -> D8, SCK -> D5, MOSI -> D7, MISO -> D6, RST -> D3
#define SS_PIN D8
#define RST_PIN D3
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

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

Servo servo;

#define HASH_SIZE 32

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-pass"
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
char pub_topic[1024];
char pub_json[1024];
char sub_accepted_topic[1024];
char sub_rejected_topic[1024];
char sub_cmd_topic[1024];
int topic_counter = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  char receivedChar[length + 1];
  for (int i=0; i < length; i++) {
    receivedChar[i] = (char)payload[i];
    Serial.print(receivedChar[i]);
  }
  receivedChar[length] = '\0';
  std::string topicString = topic;
  std::string::size_type idx = topicString.find("/cmd/request/");
  if (idx != std::string::npos) {
    // 13 is the length of /cmd/request/
    std::string cmdId = topicString.substr(idx + 13);
    Serial.print(" ... comId: ");
    Serial.println(cmdId.c_str());
    // publish a response that command received
    char cmdResTopic[1024];
    getOnenetCmdResTopic(cmdResTopic, cmdId);
    char cmdResPayload[1024];
    generateOnenetCmdResPayload(cmdResPayload, receivedChar);
    Serial.print("Publish [");
    Serial.print(cmdResTopic);
    Serial.print("]: ");
    Serial.println(cmdResPayload);
    mqttclient.publish(cmdResTopic, cmdResPayload);

    // 缺省93度
    if (strcmp(receivedChar, "开门") == 0) {
      servo.write(0);
    } else if (strcmp(receivedChar, "关门") == 0) {
      servo.write(90);
    }
  } else {
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  // 初始化LCD和DHT11
  lcd.begin(16, 2);
  lcd.backlight();
  dht.begin();

  // SG90控制线接在D0
  servo.attach(D0);

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
  Serial.print("IP address: ");
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

  client.setInsecure();
  // client.setTrustAnchors(&cert);
  setClock();

  // 配置MQTT客户端
  // mqttclient.setServer(ONENET_SERVER, ONENET_SERVER_PORT);
  mqttclient.setServer(mqtt_ip, ONENET_SERVER_PORT);
  mqttclient.setKeepAlive(60);
  mqttclient.setSocketTimeout(30);
  mqttclient.setCallback(callback);

  getOnenetPubtopic(pub_topic);
  getOnenetSubtopic(sub_accepted_topic, sub_rejected_topic);
  getOnenetCmdtopic(sub_cmd_topic);

  // RC522
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  rfid.PCD_SetAntennaGain(0x07 << 4); // Set to 48db
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}

void loop() {
  time_t now = time(nullptr);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  lcd.clear();
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
    lcd.setCursor(0, 0);
    lcd.print("Failed");
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
    Serial.print("℃, Humidity=");
    Serial.print(h);
    Serial.println("%");
  }

  if (!mqttclient.loop()) {
    Serial.println("Connecting to public OneNET mqtts broker.....");
    if (mqttclient.connect(ONENET_CLIENT_ID, ONENET_USERNAME, onenet_token)) {
      Serial.println("OneNET mqtts broker connected");

      Serial.print("Subscribe accecpted topic: ");
      Serial.print(sub_accepted_topic);
      Serial.print(" ... ");
      boolean result = mqttclient.subscribe(sub_accepted_topic, 0);
      Serial.println(result);

      Serial.print("Subscribe rejected topic: ");
      Serial.print(sub_rejected_topic);
      Serial.print(" ... ");
      result = mqttclient.subscribe(sub_rejected_topic, 0);
      Serial.println(result);

      Serial.print("Subscribe command topic: ");
      Serial.print(sub_cmd_topic);
      Serial.print(" ... ");
      result = mqttclient.subscribe(sub_cmd_topic, 0);
      Serial.println(result);

      publishTempHumi(now, t, h);
    } else {
      Serial.print("failed with state ");
      Serial.println(mqttclient.state());
    }
  } else {
    Serial.println("Connection to public OneNET mqtts broker is alive!");
    Serial.println("OneNET mqtts broker connected");
    if (topic_counter == 0) {
      publishTempHumi(now, t, h);
    }
  }
  Serial.println();
  delay(2000);
  if (topic_counter > 5) {
    topic_counter = 0;
  } else {
    topic_counter++;
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
    rfid.uid.uidByte[1] != nuidPICC[1] ||
    rfid.uid.uidByte[2] != nuidPICC[2] ||
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bytes2HexString(rfid.uid.uidByte, rfid.uid.size));
  toggleServo();
  delay(3000);
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
  // configTime(3 * 3600, 0, "ntp.ntsc.ac.cn", "cn.ntp.org.cn");
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

void getOnenetPubtopic(char pub_topic[]) {
  std::stringstream ss;
  ss << "$sys/" << ONENET_USERNAME << "/"
     << ONENET_CLIENT_ID
     << "/dp/post/json";
  char* temp = &*ss.str().begin();
  memcpy(pub_topic, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

void generateOnenetPubJson(char pub_json[], float temperature, float humidity, time_t now) {
  std::stringstream ss;
  Serial.print("id[");
  Serial.print(now);
  Serial.print("] ... ");
  ss << "{'id':" << now
     << ",'dp':{'temperature':[{'v':" << temperature << ",'t':" << now
     << "}],'humidity':[{'v':" << humidity << ",'t':" << now
     << "}]}}";
  char* temp = &*ss.str().begin();
  memcpy(pub_json, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

void publishTempHumi(time_t now, float t, float h) {
  if (isnan(t) || isnan(h)) {
    Serial.println("Data Error! Please check DHT11.");
  } else {
    Serial.print("Publish topic: ");
    Serial.print(pub_topic);
    Serial.print(" ... ");
    generateOnenetPubJson(pub_json, t, h, now);
    boolean result = mqttclient.publish(pub_topic, pub_json);
    if (result) {
      Serial.println("succeeded!");
    } else {
      Serial.println("FAILED!");
    }
  }
}

void getOnenetSubtopic(char sub_accepted_topic[], char sub_rejected_topic[]) {
  std::stringstream ss;
  ss << "$sys/" << ONENET_USERNAME << "/"
     << ONENET_CLIENT_ID
     << "/dp/post/json/accepted";
  char* temp = &*ss.str().begin();
  memcpy(sub_accepted_topic, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;

  ss << "$sys/" << ONENET_USERNAME << "/"
     << ONENET_CLIENT_ID
     << "/dp/post/json/rejected";
  temp = &*ss.str().begin();
  memcpy(sub_rejected_topic, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

void getOnenetCmdtopic(char sub_cmd_topic[]) {
  std::stringstream ss;
  ss << "$sys/" << ONENET_USERNAME << "/"
     << ONENET_CLIENT_ID
     << "/cmd/request/#";
  char* temp = &*ss.str().begin();
  memcpy(sub_cmd_topic, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

void getOnenetCmdResTopic(char sub_cmd_res_topic[], std::string cmdId) {
  std::stringstream ss;
  ss << "$sys/" << ONENET_USERNAME << "/"
     << ONENET_CLIENT_ID
     << "/cmd/response/"
     << cmdId;
  char* temp = &*ss.str().begin();
  memcpy(sub_cmd_res_topic, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

void generateOnenetCmdResPayload(char cmdResPayload[], char receivedChar[]) {
  std::stringstream ss;
  ss << "Command [" << receivedChar << "]"
     << " received.";
  if (strcmp(receivedChar, "度数") == 0) {
    ss << "度数: " << servo.read() << ".";
  }
  char* temp = &*ss.str().begin();
  memcpy(cmdResPayload, temp, strlen(temp) + 1);
  ss.str("");
  *temp = 0;
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void toggleServo() {
  Serial.print(" 度数：");
  lcd.setCursor(0, 1);
  if (servo.read() == 93) {
    servo.write(0);
    Serial.println("0。开门了。");
    lcd.print("Opened");
  } else {
    servo.write(93);
    Serial.println("93。关门了。");
    lcd.print("Closed");
  }
  Serial.println();
}

/**
 * Convert a byte array to a HEX string
 */
String bytes2HexString(byte *buffer, byte length) {
  String hexString = "";
  for (int i = 0; i < length; i++) {
    if (buffer[i] < 0x10) {
      hexString += '0';
    }
    hexString += String(buffer[i], HEX);
    if (i < length - 1)
      hexString += '-';
  }
  hexString.toUpperCase();
  return hexString;
}
```

<br/>

代码第20-21行，引入SPI库和MFRC522库：

```c++
#include <SPI.h>
#include <MFRC522.h>
```

第36-40行，定义了一个全局MFRC522对象：

```c++
// for ESP8266, SDA(SS) -> D8, SCK -> D5, MOSI -> D7, MISO -> D6, RST -> D3
#define SS_PIN D8
#define RST_PIN D3
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;
```

<br/>

代码第326-336行，初始化和启用MFRC522，并把天线功率设置为最大的48分贝：

```c++
  // RC522
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  rfid.PCD_SetAntennaGain(0x07 << 4); // Set to 48db
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
```

<br/>

第408-459行，在读卡器检测到RFID卡或标签时，电机转动到93度（关门）或0度（开门）：

```c++
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
    rfid.uid.uidByte[1] != nuidPICC[1] ||
    rfid.uid.uidByte[2] != nuidPICC[2] ||
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bytes2HexString(rfid.uid.uidByte, rfid.uid.size));
  toggleServo();
  delay(3000);
```

<br/>



### 参考资料

1. ESP8266 and RFID-RC522 module example: [http://www.esp8266learning.com/esp8266-rfid-rc522-module-example.php)
