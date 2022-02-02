#undef __STRICT_ANSI__
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <strings.h>
#include <stdlib.h>

class NTPGPSSpooferConfiguration {
    private:
        char _config_file[32];
        char _ap_name[32] = "NTPGPSSpooferAP";
        char _ap_password[64] = "";
        char _hostname[32] = "NTPGPSSpoofer";
        char _timezone_definition[65] = "PST8PDT,M3.2.0,M11.1.0";
        bool _use_gmt = false;
        double _latitude = 3800.00;
        double _longitude = -12100.00;
        bool _heartbeat = true;

        const char *latLongToGPSString(char *, double latLong, bool isLat);

    public:
        NTPGPSSpooferConfiguration(const char *configFile = "config.json") {
            LittleFS.begin();
            strlcpy(_config_file, configFile, sizeof(_config_file));
        }

        void loadConfiguration();
        void saveConfiguration();

        const char *getApName() { return _ap_name; }
        void setApName(const char *ap_name) { strlcpy(_ap_name, ap_name, sizeof(_ap_name)); }
        const char *getApPassword() { return _ap_password; }
        void setApPassword(const char *ap_password) { strlcpy(_ap_password, ap_password, sizeof(_ap_password)); }
         const char *getHostname() { return _hostname; }
        void setHostname(const char *hostame) { strlcpy(_hostname, hostame, sizeof(_hostname)); }
        const char *getTzDefinition() { return _timezone_definition; }
        void setTzDefinition(const char *tz_definition) { strlcpy(_timezone_definition, tz_definition, sizeof(_timezone_definition)); }
        bool getUseGmt() { return _use_gmt; }
        void setUseGmt(bool useGmt) { _use_gmt = useGmt; }
        double getLatitude() { return _latitude; }
        const char *getLatitudeAsString() { static char buffer[32]; return latLongToGPSString(buffer, _latitude, true); }
        void setLatitude(double latitude) { _latitude = latitude; }
        double getLongitude() { return _longitude; }
        const char *getLongitudeAsString() { static char buffer[32]; return latLongToGPSString(buffer, _longitude, false); }
         void setLongitude(double longitude) { _longitude = longitude; }
        bool getHeartbeat() { return _heartbeat; }
        void setHeartbeat(boolean heartbeat) { _heartbeat = heartbeat; }
};
