#include <Configuration.h>
#include <math.h>

const char * NTPGPSSpooferConfiguration::latLongToGPSString(char *buffer, double latLong, bool isLat) {
    char ewns = isLat ? (latLong < 0 ? 'S' : 'N') : (latLong < 0 ? 'W' : 'E');
    latLong = fabs(latLong);
    double degrees;
    double minutes = modf(latLong, &degrees) * 60.0;

    dtostrf(degrees, 1, 0, buffer);
    char *cp = buffer + strlen(buffer);
    if (minutes < 10)
        *cp++ = '0';
    dtostrf(minutes, 1, 6, cp);
    cp = buffer + strlen(buffer);
    *cp++ = ',';
    *cp++ = ewns;
    *cp++ = '\0';
    return buffer;
}

void NTPGPSSpooferConfiguration::loadConfiguration() {
    StaticJsonDocument<512> doc;

    File configFile = LittleFS.open(_config_file, "r");
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
        Serial.println(F("Failed to read file, using default configuration"));
    else
        Serial.println(F("Successfully loaded configuration"));

    setApName(doc["ap_name"] | "NTPGPSSpooferAP");
    setApPassword(doc["ap_password"] | "");
    setHostname(doc["hostname"] | "NTPGPSSpoofer");
    setTzDefinition(doc["timezone_definition"] | "GMT0");
    setUseGmt(doc["use_gmt"] | true);
    setLatitude(doc["latitude"] | 0.0);
    setLongitude(doc["longitude"] | 0.0);
    setHeartbeat(doc["heartbeat"] | true);

    configFile.close();
}

void NTPGPSSpooferConfiguration::saveConfiguration() {
    StaticJsonDocument<512> doc;

    doc["ap_name"] = getApName();
    doc["ap_password"] = getApPassword();
    doc["hostname"] = getHostname();
    doc["timezone_definition"] = getTzDefinition();
    doc["use_gmt"] = getUseGmt();
    doc["latitude"] = getLatitude();
    doc["longitude"] = getLongitude();
    doc["heartbeat"] = getHeartbeat();

    LittleFS.remove(_config_file);
    File configFile = LittleFS.open(_config_file, "w");

    if (serializeJson(doc, configFile) == 0)
        Serial.println("Unable to save configuration");
    else
        Serial.println("Configuration saved successfully");

    serializeJsonPretty(doc, Serial);

    configFile.close();
}


