# 零售示例: 基于RFID的自动售货机

基于RFID的自动售货机(RFID-Operated Vendering Marchines，简称rovem)，由两个柜子和一个库房组成，如下图所示：
![esp8266_rovem_totalset](images/esp8266_rovem_totalset.jpg)

其中，柜子是：
![esp8266_rovem_cabinet](images/esp8266_rovem_cabinet_pad.jpg)

库房是：
![esp8266_rovem_facility](images/esp8266_rovem_facility_pad.jpg)

### Preparation
**Hardwires:**

| Name | Pieces |
| :--- | ---: | 
| ESP8266 Development Board | 3 |
| DHT11 | 3 |
| LCD1602 with I2C | 3 |
| Breadboard | 6 |
| RFID-RC522 | 6 |
| SG90(Micro Servo 9g) | 3 |
| RFID Labels (i.e. NTAG213 2x1cm) | 20 |
| Dominoes (4.4x2.2x0.82cm) | 14 |
| Plastic Box (6.4x4.7x3.7cm) | 1 |
| Wood Boxes (7x4.8x17.5cm) | 2 |
| RFID Tags | 5 |
| LM393-A3144-Hall Board (2.93x1.38cm) | 2 |
| Magnet (Φ5mmx2mm) | 2 |
   
<br/>

**Softwares:**
* Arduino
* JDK
* Apache OFBiz
* The following components deploy in OFBiz:
  * sand-rfid
  * sand-onenet
  * sand-workbench

<br/>

**MQTT Services:**
* OneNET MQTT

<br/>

### Contents
* [Rovem Cabinet]()
