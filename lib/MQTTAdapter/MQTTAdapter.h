#pragma once
#include "MQTTCommon.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <queue>
#include <string>
#include <functional>

// Callback function types for loose coupling
typedef std::function<void(const EventSchema&)> EventCallback;
typedef std::function<void(bool)> ConnectionCallback;

class MQTTAdapter {
public:
    MQTTAdapter(const char* server, int port, const char* clientId, 
                MQTTConnectionType connType = MQTTConnectionType::TCP);
    void begin(const char* username = nullptr, const char* password = nullptr);
    void subscribeTo(const char* topic);
    void loop(); // Non-blocking, call every frame
    bool isConnected();
    void publish(const char* topic, const char* payload);
    
    // WebSocket specific (for WSS mode)
    void setWebSocketPath(const char* path);  // Default: "/mqtt"
    
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
    WiFiClientSecure wifiClientSecure;
    PubSubClient mqttClient;
    MQTTConnectionType connectionType;
    const char* server;
    int port;
    const char* clientId;
    const char* username;
    const char* password;
    std::string subscriptionTopic;
    std::string wsPath;  // WebSocket path (e.g., "/mqtt")
    
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