#ifndef MQTT_ADAPTER_ESP_H
#define MQTT_ADAPTER_ESP_H

#include "MQTTCommon.h"
#include <string>
#include <queue>
#include <functional>
#include "mqtt_client.h"

class MQTTAdapterESP {
public:
    using EventCallback = std::function<void(const EventSchema&)>;
    using ConnectionCallback = std::function<void(bool)>;

    MQTTAdapterESP(const char* server, int port, const char* clientId, 
                   MQTTConnectionType connType = MQTTConnectionType::TCP);
    ~MQTTAdapterESP();

    void begin(const char* username = nullptr, const char* password = nullptr);
    void loop();
    void subscribeTo(const char* topic);
    void publish(const char* topic, const char* payload);
    
    bool isConnected();
    bool hasMessages();
    EventSchema getNextEvent();
    
    void onEvent(EventCallback callback);
    void onConnectionChange(ConnectionCallback callback);
    
    void setWebSocketPath(const char* path);
    unsigned long getLastMessageTime();
    size_t getQueueSize();
    const char* getConnectionStatus();

private:
    static void mqttEventHandler(void* handler_args, esp_event_base_t base, 
                                 int32_t event_id, void* event_data);
    void processRawMessage(const MQTTMessage& message);
    bool parseEventSchema(const std::string& jsonString, EventSchema& event);
    void notifyConnectionChange(bool connected);

    const char* server;
    int port;
    const char* clientId;
    const char* username;
    const char* password;
    MQTTConnectionType connectionType;
    std::string wsPath;
    std::string subscriptionTopic;
    
    esp_mqtt_client_handle_t mqttClient;
    
    std::queue<EventSchema> eventQueue;
    std::queue<MQTTMessage> rawMessageQueue;
    
    EventCallback eventCallback;
    ConnectionCallback connectionCallback;
    
    StaticJsonDocument<8192> jsonDoc;
    unsigned long lastMessageTime;
    bool wasConnected;
};

#endif // MQTT_ADAPTER_ESP_H
