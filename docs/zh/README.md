# flower-esp8266
ESP8266作为物联网设备，来演示商业概念。

## 基本

### 准备
**硬件：**

| 名称 | 数量 |
| :--- | ---: | 
| ESP8266开发板 | 1 |
| DHT11 | 1 |
| LCD1602(带I2C) | 1 |
| 面包板 | 2 |
| RFID-RC522 | 2 |
| SG90(Micro Servo 9g) | 1 |
| RFID卡片或标签 (符合ISO-14443标准) | 1+ |

<br/>

**软件**
* 下载并安装[Arduino开发工具](https://www.arduino.cc/en/Main/Software)

* 可选：下载并安装[Fritzing](https://fritzing.org/download/)

<br/>

**Arduino首选项**

在Arduino中，选择菜单【文件】->【首选项】->【附加开发板管理器网址】中，输入：

```
​http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

搜索并安装ESP8266版本2.7.4。<b>注意：</b>目前的代码，无法在3.0以上版本运行。

<br/>

**网络服务**

* OneNET MQTT

<br/>

### Contents
* [ESP8266测量和显示温度/湿度](esp8266-dht11-lcd1602.md) 
* [ESP8266联WiFi上网](esp8266-wifi-http.md)
* [ESP8266访问Https网页](esp8266-wifi-ssl.md)
* [ESP8266使用证书库验证和访问Https网页](esp8266-bearssl-certstore.md)
* [ESP8266实现Base64编码/解码](esp8266-base64.md)
* [ESP8266 HMac-MD5](esp8266-hmac-md5.md)
* [ESP8266 Hmac-Sha1](esp8266-hmac-sha1.md)
* [ESP8266 Hmac-Sha256](esp8266-hmac-sha256.md)
* [ESP8266实现对Bytes进行Base64编码](esp8266-base64-from-bytes.md)
* [ESP8266实现OneNET URL编码](esp8266-onenet-urlencode.md)
* [注册OneNET MQTTS产品](register-onenet-mqtts.md)
* [ESP8266生成OneNET MQTTS密码(MD5)](esp8266-onenet-mqtts-password-md5.md)
* [ESP8266生成OneNET MQTTS密码(SHA1)](esp8266-onenet-mqtts-password-sha1.md)
* [ESP8266生成OneNET MQTTS密码(SHA256)](esp8266-onenet-mqtts-password-sha256.md)
* [ESP8266连接OneNET MQTTS](esp8266-onenet-mqtts-connect.md)
* [ESP8266发布/订阅OneNET MQTTS](esp8266-onenet-mqtts-pubsub.md)
* [ESP8266接受OneNET MQTTS命令](esp8266-onenet-mqtts-command.md)
* [ESP8266 RC522读卡](esp8266-rc522-onenet-mqtts.md)
* [ESP8266 两个RC522读卡](esp8266-2rc522-onenet-mqtts.md)

<br/>

## 场景
下面的场景，是一物一码概念验证或入门培训使用。

### 零售
* [基于RFID的售货机](retail/rovem/README.md)

  [![基于RFID的售货机](retail/rovem/images/esp8266_rovem_cabinet_pad_small.jpg)](retail/rovem/README.md)

<br/>

* [基于RFID的收银机](retail/pos/README.md)

  [![基于RFID的收银机](retail/rovem/images/esp8266_rovem_facility_pad_small.jpg)](retail/pos/README.md)

<br/>

### 家居
* [基于RFID的衣柜]()

<br/>
