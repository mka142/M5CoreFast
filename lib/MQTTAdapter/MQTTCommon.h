#ifndef MQTT_COMMON_H
#define MQTT_COMMON_H

#include <string>
#include <ArduinoJson.h>

// Event schema from MQTT messages
struct EventSchema {
    std::string concertId;
    std::string eventType;
    std::string label;
    JsonObject payload;
    int position;
    unsigned long timestamp;
};

// Raw MQTT message before parsing
struct MQTTMessage {
    std::string topic;
    std::string rawPayload;
    unsigned long timestamp;
};

enum class MQTTConnectionType {
    TCP,        // mqtt:// or plain TCP
    TLS,        // mqtts:// or MQTT over TLS
    WS,         // ws:// (WebSocket)
    WSS         // wss:// (WebSocket Secure)
};

#endif // MQTT_COMMON_H
