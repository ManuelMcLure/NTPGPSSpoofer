#undef __STRICT_ANSI__
#include <defines.h>
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include <cstdlib>
#include <math.h>

#include <Configuration.h>

bool LOAD_DEFAULT_CONFIG_DATA = false;
ESP_WM_LITE_Configuration defaultConfig;

NTPGPSSpooferConfiguration *config;
ESPAsync_WiFiManager_Lite *wifiManager;

void onSTAConnected(WiFiEventStationModeConnected ipInfo)
{
  Serial.printf("Connected to %s\r\n", ipInfo.ssid.c_str());
}

// Start NTP only after IP network is connected
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
  Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
  Serial.printf("Connected: %s\r\n", WiFi.status() == WL_CONNECTED ? "yes" : "no");
  digitalWrite(LED_BUILTIN, LOW); // Turn on LED
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
  Serial.printf("Disconnected from SSID: %s\r\n", event_info.ssid.c_str());
  Serial.printf("Reason: %d\r\n", event_info.reason);
  digitalWrite(LED_BUILTIN, HIGH); // Turn off LED
}

void sendTime(time_t *timestamp)
{
  static char sendBuffer[200];
  struct tm *lt;
  int sentencelen;
  uint8_t checksum = 0;

  lt = localtime(timestamp);
  sentencelen = sprintf(sendBuffer,
                        "$GPRMC,%02d%02d%02d,A,%s,%s,0.0,0.0,%02d%02d%02d,0.0,W*",
                        lt->tm_hour,
                        lt->tm_min,
                        lt->tm_sec,
                        config->getLatitudeAsString(),
                        config->getLongitudeAsString(),
                        lt->tm_mday,
                        lt->tm_mon + 1,
                        lt->tm_year % 100);

  for (int i = 1; i < sentencelen - 1; i++)
    checksum ^= sendBuffer[i];

  sprintf(sendBuffer + sentencelen, "%02X", (unsigned int)checksum);

  Serial.println(sendBuffer);
}

String templateProcessor(const String &var)
{
  if (var == "PLACEHOLDER_HOSTNAME")
    return config->getHostname();
  else if (var == "PLACEHOLDER_WCHECKED")
    return config->getLongitude() < 0 ? "checked" : "";
  else if (var == "PLACEHOLDER_ECHECKED")
    return config->getLongitude() < 0 ? "" : "checked";
  else if (var == "PLACEHOLDER_NCHECKED")
    return config->getLatitude() < 0 ? "" : "checked";
  else if (var == "PLACEHOLDER_SCHECKED")
    return config->getLatitude() < 0 ? "checked" : "";
  else if (var == "PLACEHOLDER_LATITUDE")
    return String(abs(config->getLatitude()));
  else if (var == "PLACEHOLDER_LONGITUDE")
    return String(abs(config->getLongitude()));
  else if (var == "PLACEHOLDER_USEGMTCHECKED")
    return config->getUseGmt() ? "checked" : "";
  else if (var == "PLACEHOLDER_TIMEZONEDEFINITION")
    return config->getTzDefinition();
  else if (var == "PLACEHOLDER_HEARTBEAT")
    return config->getHeartbeat() ? "checked" : "";
  else
    return "";
}

void handleRequest(AsyncWebServerRequest *request, bool post, bool update)
{
  if (update) {
    if (request->hasParam("hostname", post)) {
      config->setHostname(request->getParam("hostname", post)->value().c_str());
    }
    if (request->hasParam("latitude", post)) {
      double latitude = std::stod(request->getParam("latitude", post)->value().c_str());
      if (request->hasParam("latitude_ns", post) && request->getParam("latitude_ns", post)->value() == "S")
        latitude = -latitude;
      config->setLatitude(latitude);
    }
    if (request->hasParam("longitude", post)) {
      double longitude = std::stod(request->getParam("longitude", post)->value().c_str());
      if (request->hasParam("longitude_ew", post) && request->getParam("longitude_ew", post)->value() == "W")
        longitude = -longitude;
      config->setLongitude(longitude);
    }
    config->setUseGmt(request->hasParam("useGMT", post));
    if (request->hasParam("timeZoneDefinition", post)) {
      config->setTzDefinition(request->getParam("timeZoneDefinition", post)->value().c_str());
    }
    config->setHeartbeat(request->hasParam("heartbeat", post));
    config->saveConfiguration();
    if (request->hasParam("submit_reset", post)) {
      LittleFS.end(); // Unmount filesystem
      ESP.restart();
    }
    request->redirect("/");
  }

  File templateFile = LittleFS.open("mainForm.html", "r");

  request->send(LittleFS, "/mainForm.html", "text/html", false, templateProcessor);
}

void setup()
{

  // put your setup code here, to run once:
  static WiFiEventHandler e1, e2, e3;
  wifiManager = new ESPAsync_WiFiManager_Lite();
  AsyncWebServer *webServer = new AsyncWebServer(80);

  Serial.begin(9600);
  LittleFS.begin();

  config = new NTPGPSSpooferConfiguration();
  config->loadConfiguration();
  // pool.ntp.org is a fallback - it should use whichever
  // NTP server is reported by DHCP as the primary
  configTime(config->getTzDefinition(), "pool.ntp.org");
  Serial.println("Initializing WiFi connection...");
  e1 = WiFi.onStationModeGotIP(onSTAGotIP); // As soon WiFi is connected, start NTP Client
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  e3 = WiFi.onStationModeConnected(onSTAConnected);
  WiFi.hostname(config->getHostname());
  wifiManager->setConfigPortal(config->getApName(), config->getApPassword());
  wifiManager->begin(config->getHostname());
  
  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { handleRequest(request, false, false); });

  webServer->on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
               { handleRequest(request, false, true); });

  webServer->on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
               { handleRequest(request, true, true); });

  webServer->begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
  static int last = 0;
  static time_t lastsecond = 0;
  time_t newsecond;
  static uint8_t heartbeat = LOW;

  wifiManager->run();

  if ((millis() - last) > 10)
  {
    last = millis();
    if ((newsecond = time(NULL)) != lastsecond)
    {
        digitalWrite(LED_BUILTIN, config->getHeartbeat() ? heartbeat : HIGH);
        heartbeat = (heartbeat == LOW) ? HIGH : LOW;
#ifdef PRINT_HEARTBEAT
      struct tm *lt = localtime(&newsecond);
      Serial.printf("%02d:%02d:%02d %s\r\n", lt->tm_hour, lt->tm_min, lt->tm_sec,
                    (heartbeat == HIGH) ? "HIGH" : "LOW");
#endif
      lastsecond = newsecond;
      if (wifiManager->getWiFiStatus())
        sendTime(&newsecond);
    }
  }
  delay(0);
}