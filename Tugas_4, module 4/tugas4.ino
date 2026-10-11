#include <ArduinoWiFiServer.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "BRO'WN CAFE";
const char* password = "browncafe";

ESP8266WebServer server(80);

const int pinLED = D1;
int nilaiPWM = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Smart Lighting</title>
</head>
<body>
  <h2>Smart Lighting</h2>

  <p>Kecerahan LED:
    <span id="nilai">0</span>
  </p>

  <input type="range" min="0" max="1023" value="0"
    style="width: 100%;"
    oninput="ubahLED(this.value)">

  <script>
    function ubahLED(nilai) {
      document.getElementById("nilai").innerHTML = nilai;

      fetch("/pwm?nilai=" + nilai);
    }
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handlePWM() {
  if (server.hasArg("nilai")) {
    nilaiPWM = server.arg("nilai").toInt();
    nilaiPWM = constrain(nilaiPWM, 0, 1023);

    analogWrite(pinLED, nilaiPWM);

    Serial.println(nilaiPWM);
  }

  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(pinLED, OUTPUT);
  analogWriteRange(1023);
  analogWrite(pinLED, 0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/pwm", handlePWM);

  server.begin();
  Serial.println("Web server aktif");
}

void loop() {
  server.handleClient();
}
