#pragma once
#include <M5CoreS3.h>

class RTCAdapter
{
public:
    void begin();
    void setLocalTime(const char *tz, const char *server1, const char *server2, const char *server3);
    tm getTime();
    String getTimeISO();
};