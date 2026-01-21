#include "MQTTAdapter.h"
#include <Arduino.h>

MQTTAdapter* MQTTAdapter::instance = nullptr;

MQTTAdapter::MQTTAdapter(const char* server, int port, const char* clientId, 
                         MQTTConnectionType connType)
    : server(server), port(port), clientId(clientId), connectionType(connType),
      mqttClient(connType == MQTTConnectionType::TCP ? wifiClient : (Client&)wifiClientSecure),
      eventCallback(nullptr), connectionCallback(nullptr),
      username(nullptr), password(nullptr), wsPath("/mqtt"),
      lastReconnectAttempt(0), lastMessageTime(0), wasConnected(false) {
    instance = this;
    
    // Configure secure client for TLS/WSS
    if (connectionType == MQTTConnectionType::TLS || connectionType == MQTTConnectionType::WSS) {
        wifiClientSecure.setInsecure();  // Skip certificate validation (for testing)
        // For production, use: wifiClientSecure.setCACert(ca_cert);
    }
    
    mqttClient.setServer(server, port);
    mqttClient.setCallback(messageCallback);
    mqttClient.setBufferSize(8192); // Increase buffer for larger JSON messages
    
    Serial.print("MQTT Adapter created - Type: ");
    if (connectionType == MQTTConnectionType::TCP) Serial.println("TCP");
    else if (connectionType == MQTTConnectionType::TLS) Serial.println("TLS");
    else if (connectionType == MQTTConnectionType::WSS) Serial.println("WebSocket Secure (WSS)");
}

void MQTTAdapter::setWebSocketPath(const char* path) {
    wsPath = std::string(path);
    Serial.print("WebSocket path set to: ");
    Serial.println(path);
}

void MQTTAdapter::begin(const char* username, const char* password) {
    this->username = username;
    this->password = password;
    Serial.println("MQTT adapter initialized.");
}

void MQTTAdapter::onEvent(EventCallback callback) {
    eventCallback = callback;
}

void MQTTAdapter::onConnectionChange(ConnectionCallback callback) {
    connectionCallback = callback;
}

void MQTTAdapter::subscribeTo(const char* topic) {
    subscriptionTopic = std::string(topic);
    if (mqttClient.connected()) {
        mqttClient.subscribe(topic);
        Serial.print("Subscribed to: ");
        Serial.println(topic);
    }
}

void MQTTAdapter::loop() {
    bool currentlyConnected = mqttClient.connected();
    
    // Handle connection state changes
    if (currentlyConnected != wasConnected) {
        wasConnected = currentlyConnected;
        notifyConnectionChange(currentlyConnected);
    }
    
    if (!currentlyConnected) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > reconnectInterval) {
            lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        mqttClient.loop(); // Non-blocking MQTT processing
    }
    
    // Process one raw message per loop iteration to maintain fluidity
    if (!rawMessageQueue.empty()) {
        MQTTMessage rawMsg = rawMessageQueue.front();
        rawMessageQueue.pop();
        processRawMessage(rawMsg);
    }
}

void MQTTAdapter::messageCallback(char* topic, byte* payload, unsigned int length) {
    if (instance) {
        std::string payloadStr((char*)payload, length);
        MQTTMessage msg = {
            std::string(topic),
            payloadStr,
            millis()
        };
        instance->rawMessageQueue.push(msg);
        instance->lastMessageTime = millis();
    }
}

void MQTTAdapter::processRawMessage(const MQTTMessage& message) {
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

bool MQTTAdapter::parseEventSchema(const std::string& jsonString, EventSchema& event) {
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

bool MQTTAdapter::hasMessages() {
    return !eventQueue.empty();
}

EventSchema MQTTAdapter::getNextEvent() {
    EventSchema event = eventQueue.front();
    eventQueue.pop();
    return event;
}

void MQTTAdapter::reconnect() {
    Serial.print("Attempting MQTT connection (");
    if (connectionType == MQTTConnectionType::TCP) Serial.print("TCP");
    else if (connectionType == MQTTConnectionType::TLS) Serial.print("TLS");
    else if (connectionType == MQTTConnectionType::WSS) Serial.print("WSS");
    Serial.print(")...");
    
    // For WSS, we need to establish WebSocket connection first
    if (connectionType == MQTTConnectionType::WSS) {
        if (!wifiClientSecure.connected()) {
            Serial.print("Connecting to WebSocket at ");
            Serial.print(server);
            Serial.print(":");
            Serial.print(port);
            Serial.print(wsPath.c_str());
            Serial.print("...");
            
            if (!wifiClientSecure.connect(server, port)) {
                Serial.println("WebSocket connection failed");
                return;
            }
            
            // Send WebSocket upgrade request
            String request = "GET " + String(wsPath.c_str()) + " HTTP/1.1\r\n";
            request += "Host: " + String(server) + "\r\n";
            request += "Upgrade: websocket\r\n";
            request += "Connection: Upgrade\r\n";
            request += "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n";
            request += "Sec-WebSocket-Protocol: mqtt\r\n";
            request += "Sec-WebSocket-Version: 13\r\n\r\n";
            
            wifiClientSecure.print(request);
            
            // Wait for upgrade response (non-blocking)
            unsigned long startTime = millis();
            const unsigned long responseTimeout = 5000; // 5 seconds
            
            // Wait for data to be available
            while (wifiClientSecure.available() == 0) {
                if (millis() - startTime > responseTimeout) {
                    Serial.println("WebSocket upgrade timeout");
                    wifiClientSecure.stop();
                    return;
                }
                // Non-blocking wait - just return and try again next loop
                yield(); // Allow other tasks to run
            }
            
            // Read HTTP response headers line by line
            // We only need to verify "101 Switching Protocols" and consume the headers
            // Stop reading when we hit the empty line (end of HTTP headers)
            bool upgradeSuccess = false;
            String line = "";
            unsigned long readStartTime = millis();
            const unsigned long readTimeout = 2000; // 2 seconds for reading response
            
            while (wifiClientSecure.available()) {
                char c = wifiClientSecure.read();
                if (c == '\n') {
                    // Check if this is the status line with "101"
                    if (line.indexOf("101") != -1) {
                        upgradeSuccess = true;
                    }
                    // Empty line means end of HTTP headers
                    if (line.length() <= 1) {  // \r or empty
                        break;
                    }
                    line = "";
                } else if (c != '\r') {
                    line += c;
                }
                // Timeout protection
                if (millis() - readStartTime > readTimeout) {
                    Serial.println("WebSocket response timeout");
                    wifiClientSecure.stop();
                    return;
                }
            }
            
            if (!upgradeSuccess) {
                Serial.println("WebSocket upgrade failed - no 101 response");
                wifiClientSecure.stop();
                return;
            }
            
            Serial.println("WebSocket connected");
        }
    }
    
    // Now connect MQTT
    bool connected = false;
    if (username && password) {
        connected = mqttClient.connect(clientId, username, password);
    } else {
        connected = mqttClient.connect(clientId);
    }
    
    if (connected) {
        Serial.println("MQTT connected");
        if (!subscriptionTopic.empty()) {
            mqttClient.subscribe(subscriptionTopic.c_str());
            Serial.print("Resubscribed to: ");
            Serial.println(subscriptionTopic.c_str());
        }
    } else {
        Serial.print("MQTT connection failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" will retry in 5 seconds");
    }
}

void MQTTAdapter::notifyConnectionChange(bool connected) {
    if (connectionCallback) {
        connectionCallback(connected);
    }
    Serial.print("MQTT connection status changed: ");
    Serial.println(connected ? "CONNECTED" : "DISCONNECTED");
}

bool MQTTAdapter::isConnected() {
    return mqttClient.connected();
}

void MQTTAdapter::publish(const char* topic, const char* payload) {
    if (mqttClient.connected()) {
        bool result = mqttClient.publish(topic, payload);
        if (!result) {
            Serial.println("Failed to publish MQTT message");
        }
    } else {
        Serial.println("Cannot publish: MQTT not connected");
    }
}

unsigned long MQTTAdapter::getLastMessageTime() {
    return lastMessageTime;
}

size_t MQTTAdapter::getQueueSize() {
    return eventQueue.size() + rawMessageQueue.size();
}

const char* MQTTAdapter::getConnectionStatus() {
    if (!mqttClient.connected()) {
        switch (mqttClient.state()) {
            case MQTT_CONNECTION_TIMEOUT: return "Connection timeout";
            case MQTT_CONNECTION_LOST: return "Connection lost";
            case MQTT_CONNECT_FAILED: return "Connect failed";
            case MQTT_DISCONNECTED: return "Disconnected";
            case MQTT_CONNECT_BAD_PROTOCOL: return "Bad protocol";
            case MQTT_CONNECT_BAD_CLIENT_ID: return "Bad client ID";
            case MQTT_CONNECT_UNAVAILABLE: return "Server unavailable";
            case MQTT_CONNECT_BAD_CREDENTIALS: return "Bad credentials";
            case MQTT_CONNECT_UNAUTHORIZED: return "Unauthorized";
            default: return "Unknown error";
        }
    }
    return "Connected";
}