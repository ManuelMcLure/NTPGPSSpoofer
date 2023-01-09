# NTPGPSSpoofer

This is a small piece of code that allows an ESP8266 to spoof being a
GPS receiver to interface with clocks that can automatically set their
time from GPS `$GPRMC` sentences.

## Features

- Automatic DST support for clocks that don't have this included.
- If WiFi connection information isn't configured, the process will start an Access Point (SSID **NTPGPSSpooferAP** by default) which you can connect to that will request the WiFi information (courtesy of the ESPAsync_WiFiManager_Lite library)
- Once connected, there will be a web page that allows configuring some of the settings, such as the time zone definition and spoofed location.

## Wiring

Provide +5V to the `Vin` pin on your ESP8266, ground to any of the `GND` pins, and connect the input signal to the clock to the `GPIO1` pin (`TX` on a NodeMCU).

## Running

I recommend testing the program before hooking it up to the clock (it's a good idea to configure WiFi as well). Use the PlatformIO monitor to see what's going on - if it shows that the ESP8266 started an access point, connect to the AP and configure WiFi. Once everything is working correctly, you should start seeing a message such as:

`$GPRMC,160045,A,3800.00,N,12100.00,W,0.0,0.0,311269,0.0,W*77`

once a second. This is the same message that is being output to `GPIO1` for the clock to consume.

Once the server is running, you can connect to the IP address of the ESP8266 (should appear in the debug output) or to hostname "NTPGPSSpoofer" (default) to configure through the web page.

## Configuration

Several variables can be configured in the web interface:

- Hostname - the hostname that will be provided to the DHCP server on your network
- Latitude and longitude - the spoofed location provided in the GPS messages
- Timezone - see below

## Setting the time zone

There are two ways to set the time zone information. Note that GPS messages only send UTC (GMT) time. It's usually up to the clock to convert this time to the local time zone before display.

If you are in a time zone that does not have Daylight Saving Time, or your clock is smart enough to handle time changes on its own, the easiest option is to check the "Use GMT/UTC" check box and set the clock's timezone appropriately. If you don't mind changing the clock yourself twice a year you can also select this option if you're in a time zone that does DST, you'll just have to adjust the clock time zone twice a year.

If, like me, you want your clock to automatically adjust the time but the clock doesn't support this, we can "trick" the clock by sending the correct local time in the GPS messages, and letting the clock think that it's displaying GMT. To do this, uncheck the "Used GMT/UTC" checkbox, and enter a time zone recipe in the "Timezone" field.

Please note that due to the limited program space on the 8266, it does not have the library of time zones that is common on Linux systems, so you can't just specify "America/Los_Angeles" or "PST8PDT" - instead you will need to specify what the rules are for the time change.

A recipe is of the form:

**STD***offset***DST**,M*m*.*w*.*d*,M*m*.*w*.*d*

Where **STD** is the three letter designation for the standard time zone (e.g. **PST**), *offset* is the standard time zone offset from GMT, **DST** is the three letter designation for the DST time zone, and *m*, *w* and *d* are the month, occurrence of the specified day within the month and day of week when the time zone changes. For example, the full definition for Pacific time is:

PST8PDT,M3,2,0,M11,1,0

This means that the standard offset will be 8, DST will start on the second (w=2) Sunday (d=0) of March (m=3), and end on the first (w=1) Sunday (d=0) of November (m=11).

Please read https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html for more details on how to set the time zone variable if using this method.