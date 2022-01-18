# NTPGPSSpoofer

This is a small piece of code that allows an ESP8266 to spoof being a
GPS receiver to interface with clocks that can automatically set their
time from GPS `$GPRMC` sentences.

## Features

- Automatic DST support for clocks that don't have this included.
- If WiFi connection information isn't configured, the process will start an Access Point (SSID **NTPGPSSpoofer** by default) which you can connect to that will request the WiFi information (courtesy of the WiFiManager library)
- The spoofed location can be changed in the code (defaults to somewhere around 60 miles southeast of Sacramento, CA, USA) in case your clock does something interesting with the location.

## Configuration

* Set the time zone
    1. If you're in a time zone that doesn't support DST or your clock handles DST correctly,
    _and_ your clock has a time zone setting (which it probably does if it accepts GPS input),
    then set the `TIMEZONE` variable to `TZ_Etc_GMT` and set the correct time zone on your clock.
    3. If you want NTPGPSSpoofer to automatically change time when DST occurs, set `TIMEZONE`
    to a valid timezone constant from `TZ.h`, for example `TZ_America_Los_Angeles`.
    Set the time zone on your clock to GMT/UTC.
* Set the spoofed position.
    The format for the `latitude` and `longitude` variables is
    `"DDMM.MMMM,[E|W|N|S]"`
    i.e. degrees * 100 + minutes and fractions. For example, to represent longitude 121 degrees, 56.1234 minutes West use

    `static const char *longitude = "12156.1234,W";`

## Wiring

Provide +5V to the `Vin` pin on your ESP8266, ground to any of the `GND` pins, and connect the input signal to the clock to the `GPIO2` pin (`D4` on a NodeMCU).

## Running

I recommend testing the program before hooking it up to the clock (it's a good idea to configure WiFi as well). Use the PlatformIO monitor to see what's going on - if it shows that the ESP8266 started an access point, connect to the AP and configure WiFi. Once everything is working correctly, you should start seeing a message such as:

`$GPRMC,160045,A,3800.00,N,12100.00,W,0.0,0.0,311269,0.0,W*77`

once a second. This is the same message that is being output to `GPIO2` for the clock to consume.
