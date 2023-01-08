# NTPGPSSpoofer

This is a small piece of code that allows an ESP8266 to spoof being a
GPS receiver to interface with clocks that can automatically set their
time from GPS `$GPRMC` sentences.

## Features

- Automatic DST support for clocks that don't have this included.
- If WiFi connection information isn't configured, the process will start an Access Point (SSID **NTPGPSSpooferAP** by default) which you can connect to that will request the WiFi information (courtesy of the ESPAsync_WiFiManager_Lite library)
- Once connected, there will be a web page that allows configuring some of the settings, such as the time zone definition and spoofed location.

## Wiring

Provide +5V to the `Vin` pin on your ESP8266, ground to any of the `GND` pins, and connect the input signal to the clock to the `GPIO2` pin (`D4` on a NodeMCU).

## Running

I recommend testing the program before hooking it up to the clock (it's a good idea to configure WiFi as well). Use the PlatformIO monitor to see what's going on - if it shows that the ESP8266 started an access point, connect to the AP and configure WiFi. Once everything is working correctly, you should start seeing a message such as:

`$GPRMC,160045,A,3800.00,N,12100.00,W,0.0,0.0,311269,0.0,W*77`

once a second. This is the same message that is being output to `GPIO2` for the clock to consume.

Once the server is running, you can connect to the IP address of the ESP8266 (should appear in the debug output) or to hostname "NTPGPSSpoofer" (default) to configure through the web page.
