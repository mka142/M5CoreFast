#include "WiFiAdapter.h"

void WiFiAdapter::begin(const char *ssid, const char *password)
{
    _ssid = ssid;
    _password = password;
    WiFi.begin(_ssid, _password);
    _lastAttempt = millis();
    Serial.println("WiFi adapter initialized.");
}

bool WiFiAdapter::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void WiFiAdapter::loop()
{
    if (!isConnected() && millis() - _lastAttempt > _retryInterval)
    {
        WiFi.disconnect();
        WiFi.begin(_ssid, _password);
        _lastAttempt = millis();
    }
}