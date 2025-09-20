#include "RTCAdapter.h"
#include <WiFi.h>
#include <esp_sntp.h>

void RTCAdapter::begin()
{

    if (!CoreS3.Rtc.isEnabled())
    {
        Serial.println("RTC not found.");
        while (true)
        {
            delay(500);
        }
    }
    Serial.println("RTC adapter initialized.");
}

void RTCAdapter::setLocalTime(const char *tz, const char *server1, const char *server2, const char *server3)
{
    configTzTime(tz, server1, server2, server3);
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED)
    {
        delay(1000);
    }
    time_t t = time(nullptr) + 1;
    while (t > time(nullptr))
        ;
    CoreS3.Rtc.setDateTime(gmtime(&t));
    Serial.println("RTC local time set.");
}

tm RTCAdapter::getTime()
{
    struct tm timeInfo;
    getLocalTime(&timeInfo, 1000);
    return timeInfo;
}

String RTCAdapter::getTimeISO()
{
    struct tm timeInfo;
    getLocalTime(&timeInfo, 1000);
    char buf[32];
    // Format date and time as ISO 8601 string: YYYY-MM-DDTHH:MM:SS
    sprintf(buf, "%04d-%02d-%02dT%02d:%02d:%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
            timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
    return String(buf);
}