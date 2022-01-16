#undef __STRICT_ANSI__
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>
#include <DNSServer.h>

#include <ESP8266WebServer.h>
#include <ESP8266WebServerSecure.h>

#include <WiFiManager.h>
#include <time.h>
#include <cstdlib>
#include <TZ.h>

// Set to your timezone as defined in TZ.h
#define TIMEZONE  TZ_America_Los_Angeles
// Host name and AP name for WIFi configuration
#define HOSTNAME  "NTPGPSSpoofer"
#define AP_SSID   "NTPGPSSpoofer"

// This is the faked location. The format is
// DDMM.MMMMMMMM,[E|W|N|S]
// i.e. degrees, minutes, fractional minutes.
// For example,
// 12146.12345,E
// would be 121 degrees, 45.12345 minutes East
static const char *latitude = "3800.00,N";
static const char *longitude = "12100.00,W";

bool wifiFirstConnected = true;
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
  wifiFirstConnected = true;
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
  Serial.printf("Disconnected from SSID: %s\r\n", event_info.ssid.c_str());
  Serial.printf("Reason: %d\r\n", event_info.reason);
  digitalWrite(LED_BUILTIN, HIGH); // Turn off LED
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void sendTime(time_t *timestamp) {
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
                        latitude,
                        longitude,
                        lt->tm_mday,
                        lt->tm_mon + 1,
                        lt->tm_year % 100);

  for (int i = 1; i < sentencelen - 1; i++)
    checksum ^= sendBuffer[i];

  sprintf(sendBuffer + sentencelen, "%02X", (unsigned int)checksum);
  
  Serial.println(sendBuffer);
  Serial1.println(sendBuffer);
}

void setup() {
  // put your setup code here, to run once:
  static WiFiEventHandler e1, e2, e3;
  WiFiManager wifiManager;

  Serial.begin(115200);
  Serial1.begin(9600);
  // pool.ntp.org is a fallback - it should use whichever
  // NTP server is reported by DHCP as the primary
  configTime(TIMEZONE, "pool.ntp.org");
  Serial.println("Initializing WiFi connection...");
  e1 = WiFi.onStationModeGotIP(onSTAGotIP); // As soon WiFi is connected, start NTP Client
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  e3 = WiFi.onStationModeConnected(onSTAConnected);
  WiFi.hostname(HOSTNAME);
  wifiManager.setHostname(HOSTNAME);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect(AP_SSID);
}

void loop() {
  // put your main code here, to run repeatedly:
  static int last = 0;
  static time_t lastsecond = 0;
  time_t newsecond;
  static uint8_t heartbeat = LOW;
 
  if (wifiFirstConnected)
  {
    wifiFirstConnected = false;
  }

  if ((millis() - last) > 10)
  {
    last = millis();
    if ((newsecond = time(NULL)) != lastsecond)
    {
      digitalWrite(LED_BUILTIN, heartbeat);
#ifdef PRINT_HEARTBEAT
      struct tm *lt = localtime(&newsecond);
      Serial.printf("%02d:%02d:%02d %s\r\n", lt->tm_hour, lt->tm_min, lt->tm_sec,
        (heartbeat == HIGH) ? "HIGH" : "LOW");
#endif
      heartbeat = (heartbeat == LOW) ? HIGH : LOW;
      lastsecond = newsecond;
      sendTime(&newsecond);
    }
  }
  delay(0);
}