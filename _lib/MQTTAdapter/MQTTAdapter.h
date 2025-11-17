#pragma once
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <queue>
#include <string>
#include <functional>

struct EventSchema {
    std::string concertId;
    std::string eventType;
    std::string label;
    JsonObject payload;
    int position;
    unsigned long timestamp; // Added for internal tracking
};

struct MQTTMessage {
    std::string topic;
    std::string rawPayload;
    unsigned long timestamp;
};

// Callback function types for loose coupling
typedef std::function<void(const EventSchema&)> EventCallback;
typedef std::function<void(bool)> ConnectionCallback;

class MQTTAdapter {
public:
    MQTTAdapter(const char* server, int port, const char* clientId);
    void begin(const char* username = nullptr, const char* password = nullptr);
    void subscribeTo(const char* topic);
    void loop(); // Non-blocking, call every frame
    bool isConnected();
    void publish(const char* topic, const char* payload);
    
    // Callback registration for loose coupling
    void onEvent(EventCallback callback);
    void onConnectionChange(ConnectionCallback callback);
    
    // Message queue management
    bool hasMessages();
    EventSchema getNextEvent();
    
    // Status and diagnostics
    unsigned long getLastMessageTime();
    size_t getQueueSize();
    const char* getConnectionStatus();
    
private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    const char* server;
    int port;
    const char* clientId;
    const char* username;
    const char* password;
    std::string subscriptionTopic;
    
    // Message processing
    std::queue<EventSchema> eventQueue;
    std::queue<MQTTMessage> rawMessageQueue;
    
    // Callbacks for loose coupling
    EventCallback eventCallback;
    ConnectionCallback connectionCallback;
    
    // Connection management
    unsigned long lastReconnectAttempt;
    unsigned long lastMessageTime;
    const unsigned long reconnectInterval = 5000; // 5 seconds
    bool wasConnected;
    
    // JSON processing
    StaticJsonDocument<1024> jsonDoc; // Adjust size based on your message size
    
    // Callbacks
    static void messageCallback(char* topic, byte* payload, unsigned int length);
    void processRawMessage(const MQTTMessage& message);
    bool parseEventSchema(const std::string& jsonString, EventSchema& event);
    void reconnect();
    void notifyConnectionChange(bool connected);
    
    static MQTTAdapter* instance; // For static callback
};