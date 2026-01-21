#ifndef EVENT_POLLER_H
#define EVENT_POLLER_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <functional>
#include <MQTTCommon.h>

/**
 * EventPoller - Polls HTTP endpoint for current concert event
 * 
 * Polls the API every N seconds to check for event changes.
 * Notifies callbacks when a new event is detected.
 */
class EventPoller {
public:
    using EventCallback = std::function<void(const EventSchema&)>;
    
    EventPoller(const char* apiEndpoint, unsigned long pollIntervalMs = 5000);
    
    void begin();
    void loop();
    void onEvent(EventCallback callback);
    
    unsigned long getLastPollTime() { return lastPollTime; }
    unsigned long getLastSuccessTime() { return lastSuccessTime; }
    bool hasError() { return errorState; }
    const char* getLastError() { return lastError.c_str(); }
    
private:
    const char* endpoint;
    unsigned long pollInterval;
    unsigned long lastPollTime;
    unsigned long lastSuccessTime;
    
    EventCallback eventCallback;
    
    std::string lastEventId;
    bool errorState;
    std::string lastError;
    
    HTTPClient http;
    JsonDocument jsonDoc;  // Using new JsonDocument API
    
    void pollEndpoint();
    bool parseEventResponse(const String& response, EventSchema& event);
};

#endif // EVENT_POLLER_H
