#include "MQTTAdapterESP.h"
#include <Arduino.h>

MQTTAdapterESP::MQTTAdapterESP(const char* server, int port, const char* clientId, 
                               MQTTConnectionType connType)
    : server(server), port(port), clientId(clientId), connectionType(connType),
      eventCallback(nullptr), connectionCallback(nullptr),
      username(nullptr), password(nullptr), wsPath("/mqtt"),
      lastMessageTime(0), wasConnected(false), mqttClient(nullptr) {
    
    Serial.print("MQTT Adapter (ESP-IDF) created - Type: ");
    if (connectionType == MQTTConnectionType::TCP) Serial.println("TCP");
    else if (connectionType == MQTTConnectionType::TLS) Serial.println("TLS");
    else if (connectionType == MQTTConnectionType::WS) Serial.println("WebSocket (WS)");
    else if (connectionType == MQTTConnectionType::WSS) Serial.println("WebSocket Secure (WSS)");
}

MQTTAdapterESP::~MQTTAdapterESP() {
    if (mqttClient) {
        esp_mqtt_client_stop(mqttClient);
        esp_mqtt_client_destroy(mqttClient);
    }
}

void MQTTAdapterESP::setWebSocketPath(const char* path) {
    wsPath = std::string(path);
    Serial.print("WebSocket path set to: ");
    Serial.println(path);
}

void MQTTAdapterESP::begin(const char* username, const char* password) {
    this->username = username;
    this->password = password;
    
    // Build the broker URI based on connection type
    String uri;
    switch (connectionType) {
        case MQTTConnectionType::TCP:
            uri = "mqtt://";
            break;
        case MQTTConnectionType::TLS:
            uri = "mqtts://";
            break;
        case MQTTConnectionType::WS:
            uri = "ws://";
            break;
        case MQTTConnectionType::WSS:
            uri = "wss://";
            break;
    }
    
    uri += server;
    uri += ":";
    uri += String(port);
    
    // Add WebSocket path for WS/WSS
    if (connectionType == MQTTConnectionType::WS || connectionType == MQTTConnectionType::WSS) {
        uri += wsPath.c_str();
    }
    
    Serial.print("Connecting to MQTT broker: ");
    Serial.println(uri);
    
    // Configure MQTT client (using older API compatible with Arduino ESP32)
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.uri = uri.c_str();
    mqtt_cfg.client_id = clientId;
    
    if (username && password) {
        mqtt_cfg.username = username;
        mqtt_cfg.password = password;
    }
    
    // For WSS/TLS, skip certificate verification (for testing)
    if (connectionType == MQTTConnectionType::WSS || connectionType == MQTTConnectionType::TLS) {
        mqtt_cfg.skip_cert_common_name_check = true;
        mqtt_cfg.cert_pem = nullptr; // Skip cert validation
    }
    
    // Increase buffer size for larger messages
    mqtt_cfg.buffer_size = 8192;
    mqtt_cfg.out_buffer_size = 8192;
    
    // Create MQTT client
    mqttClient = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqttClient) {
        Serial.println("Failed to initialize MQTT client");
        return;
    }
    
    // Register event handler
    esp_mqtt_client_register_event(mqttClient, MQTT_EVENT_ANY, mqttEventHandler, this);
    
    // Start MQTT client
    esp_err_t err = esp_mqtt_client_start(mqttClient);
    if (err != ESP_OK) {
        Serial.print("Failed to start MQTT client: ");
        Serial.println(err);
    } else {
        Serial.println("MQTT adapter initialized.");
    }
}

void MQTTAdapterESP::onEvent(EventCallback callback) {
    eventCallback = callback;
}

void MQTTAdapterESP::onConnectionChange(ConnectionCallback callback) {
    connectionCallback = callback;
}

void MQTTAdapterESP::subscribeTo(const char* topic) {
    subscriptionTopic = std::string(topic);
    if (mqttClient && isConnected()) {
        int msg_id = esp_mqtt_client_subscribe(mqttClient, topic, 0);
        Serial.print("Subscribed to: ");
        Serial.print(topic);
        Serial.print(", msg_id=");
        Serial.println(msg_id);
    }
}

void MQTTAdapterESP::loop() {
    // ESP-IDF MQTT client runs in its own task, so we just process queued messages
    bool currentlyConnected = isConnected();
    
    // Handle connection state changes
    if (currentlyConnected != wasConnected) {
        wasConnected = currentlyConnected;
        notifyConnectionChange(currentlyConnected);
    }
    
    // Process one raw message per loop iteration to maintain fluidity
    if (!rawMessageQueue.empty()) {
        MQTTMessage rawMsg = rawMessageQueue.front();
        rawMessageQueue.pop();
        processRawMessage(rawMsg);
    }
}

void MQTTAdapterESP::mqttEventHandler(void* handler_args, esp_event_base_t base, 
                                      int32_t event_id, void* event_data) {
    MQTTAdapterESP* adapter = static_cast<MQTTAdapterESP*>(handler_args);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    
    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            Serial.println("MQTT connected");
            // Resubscribe to topic
            if (!adapter->subscriptionTopic.empty()) {
                int msg_id = esp_mqtt_client_subscribe(adapter->mqttClient, 
                                                       adapter->subscriptionTopic.c_str(), 0);
                Serial.print("Resubscribed to: ");
                Serial.print(adapter->subscriptionTopic.c_str());
                Serial.print(", msg_id=");
                Serial.println(msg_id);
            }
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            Serial.println("MQTT disconnected");
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            Serial.print("MQTT_EVENT_SUBSCRIBED, msg_id=");
            Serial.println(event->msg_id);
            break;
            
        case MQTT_EVENT_DATA:
            // Message received
            {
                std::string topic(event->topic, event->topic_len);
                std::string payload(event->data, event->data_len);
                
                MQTTMessage msg = {
                    topic,
                    payload,
                    millis()
                };
                adapter->rawMessageQueue.push(msg);
                adapter->lastMessageTime = millis();
                
                Serial.print("MQTT message received on topic: ");
                Serial.println(topic.c_str());
            }
            break;
            
        case MQTT_EVENT_ERROR:
            Serial.println("MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                Serial.println("TCP transport error");
            }
            break;
            
        default:
            break;
    }
}

void MQTTAdapterESP::processRawMessage(const MQTTMessage& message) {
    Serial.print("Processing MQTT message from topic: ");
    Serial.println(message.topic.c_str());
    Serial.print("Raw payload: ");
    Serial.println(message.rawPayload.c_str());
    
    EventSchema event;
    if (parseEventSchema(message.rawPayload, event)) {
        event.timestamp = message.timestamp;
        eventQueue.push(event);
        
        // Notify callback if registered
        if (eventCallback) {
            eventCallback(event);
        }
        
        Serial.print("Successfully parsed event - Type: ");
        Serial.print(event.eventType.c_str());
        Serial.print(", Concert ID: ");
        Serial.print(event.concertId.c_str());
        Serial.print(", Position: ");
        Serial.println(event.position);
    } else {
        Serial.println("Failed to parse EventSchema from JSON");
    }
}

bool MQTTAdapterESP::parseEventSchema(const std::string& jsonString, EventSchema& event) {
    // Clear previous document
    jsonDoc.clear();
    
    // Parse JSON
    DeserializationError error = deserializeJson(jsonDoc, jsonString);
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Extract required fields
    if (!jsonDoc["concertId"].is<const char*>() || 
        !jsonDoc["eventType"].is<const char*>() || 
        !jsonDoc["label"].is<const char*>() || 
        !jsonDoc["payload"].is<JsonObject>() || 
        !jsonDoc["position"].is<int>()) {
        Serial.println("Missing required fields in EventSchema");
        return false;
    }
    
    // Populate EventSchema
    event.concertId = jsonDoc["concertId"].as<std::string>();
    event.eventType = jsonDoc["eventType"].as<std::string>();
    event.label = jsonDoc["label"].as<std::string>();
    event.payload = jsonDoc["payload"].as<JsonObject>();
    event.position = jsonDoc["position"].as<int>();
    
    return true;
}

bool MQTTAdapterESP::hasMessages() {
    return !eventQueue.empty();
}

EventSchema MQTTAdapterESP::getNextEvent() {
    EventSchema event = eventQueue.front();
    eventQueue.pop();
    return event;
}

void MQTTAdapterESP::notifyConnectionChange(bool connected) {
    if (connectionCallback) {
        connectionCallback(connected);
    }
    Serial.print("MQTT connection status changed: ");
    Serial.println(connected ? "CONNECTED" : "DISCONNECTED");
}

bool MQTTAdapterESP::isConnected() {
    // Check if client is initialized and connected
    return mqttClient != nullptr;
    // Note: ESP-IDF MQTT doesn't have a simple isConnected() API
    // Connection state is tracked through events
}

void MQTTAdapterESP::publish(const char* topic, const char* payload) {
    if (mqttClient) {
        int msg_id = esp_mqtt_client_publish(mqttClient, topic, payload, 0, 0, 0);
        if (msg_id < 0) {
            Serial.println("Failed to publish MQTT message");
        }
    } else {
        Serial.println("Cannot publish: MQTT not initialized");
    }
}

unsigned long MQTTAdapterESP::getLastMessageTime() {
    return lastMessageTime;
}

size_t MQTTAdapterESP::getQueueSize() {
    return eventQueue.size() + rawMessageQueue.size();
}

const char* MQTTAdapterESP::getConnectionStatus() {
    if (mqttClient) {
        return wasConnected ? "Connected" : "Connecting/Disconnected";
    }
    return "Not initialized";
}
