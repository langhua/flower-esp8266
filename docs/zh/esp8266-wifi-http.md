# ESP8266联WiFi上网

在Arduino IDE中，选择菜单【文件】->【示例】->【ESP8266HttpClient】->【Authorization】，如下图所示：

![ESP8266-HttpClient-Authorization](images/ssl/ESP8266-HttpClient-Authorization_small.png)

<br/>

出现的代码文件如下：

```c++
/**
   Authorization.ino

    Created on: 09.12.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASSWORD");

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url


    http.begin(client, "http://guest:guest@jigsaw.w3.org/HTTP/Basic/");

    /*
      // or
      http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("guest", "guest");

      // or
      http.begin(client, "http://jigsaw.w3.org/HTTP/Basic/");
      http.setAuthorization("Z3Vlc3Q6Z3Vlc3Q=");
    */


    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(10000);
}
```


<br/>

在Arduino IDE中，修改上面这段程序的第35行：

```c++
  WiFiMulti.addAP("SSID", "PASSWORD");
```

<br/>

修改为你的路由器ID和密码，比如：

```c++
  WiFiMulti.addAP("CU_yHg6", "24ty6ycr");
```

<br/>

然后运行这个程序，可以在串口监视器中看到下图中右侧窗口的数据；打开浏览器，访问[http://guest:guest@jigsaw.w3.org/HTTP/Basic/](http://guest:guest@jigsaw.w3.org/HTTP/Basic/)，页面内容如下图左侧所示；查看页面源代码，内容如下图中间所示：

![ESP8266-WiFi-Authorization_browser](images/ssl/ESP8266-WiFi-Authorization_browser.png)

<br/>

**至此，ESP8266通过连接WiFi路由器，成功访问了互联网上的网站。**

<br/>