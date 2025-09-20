#pragma once
#include <WiFi.h>

class WiFiAdapter
{
public:
    void begin(const char *ssid, const char *password);
    bool isConnected();
    void loop();

private:
    const char *_ssid;
    const char *_password;
    unsigned long _lastAttempt = 0;
    const unsigned long _retryInterval = 30000; // 30 seconds
};