#include "EventPoller.h"
#include <Arduino.h>

EventPoller::EventPoller(const char* apiEndpoint, unsigned long pollIntervalMs)
    : endpoint(apiEndpoint), pollInterval(pollIntervalMs),
      lastPollTime(0), lastSuccessTime(0), eventCallback(nullptr),
      errorState(false) {
}

void EventPoller::begin() {
    Serial.println("Event Poller initialized");
    Serial.print("Endpoint: ");
    Serial.println(endpoint);
    Serial.print("Poll interval: ");
    Serial.print(pollInterval / 1000);
    Serial.println(" seconds");
}

void EventPoller::onEvent(EventCallback callback) {
    eventCallback = callback;
}

void EventPoller::loop() {
    unsigned long now = millis();
    
    // Check if it's time to poll
    if (now - lastPollTime >= pollInterval) {
        lastPollTime = now;
        pollEndpoint();
    }
}

void EventPoller::pollEndpoint() {
    Serial.print("Polling event endpoint... ");
    
    http.begin(endpoint);
    http.setTimeout(5000); // 5 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
        Serial.print("HTTP ");
        Serial.println(httpCode);
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            lastSuccessTime = millis();
            errorState = false;
            
            Serial.print("Response length: ");
            Serial.println(payload.length());
            
            EventSchema event;
            if (parseEventResponse(payload, event)) {
                // Check if this is a new event (different from last)
                if (event.concertId != lastEventId) {
                    lastEventId = event.concertId;
                    Serial.println("New event detected!");
                    Serial.print("Event ID: ");
                    Serial.println(event.concertId.c_str());
                    Serial.print("Event Type: ");
                    Serial.println(event.eventType.c_str());
                    
                    // Notify callback
                    if (eventCallback) {
                        eventCallback(event);
                    }
                } else {
                    Serial.println("Same event as before (no change)");
                }
            } else {
                errorState = true;
                lastError = "Failed to parse event response";
                Serial.println(lastError.c_str());
            }
        } else {
            errorState = true;
            lastError = std::string("HTTP error: ") + String(httpCode).c_str();
            Serial.println(lastError.c_str());
        }
    } else {
        errorState = true;
        lastError = std::string("HTTP request failed: ") + http.errorToString(httpCode).c_str();
        Serial.println(lastError.c_str());
    }
    
    http.end();
}

bool EventPoller::parseEventResponse(const String& response, EventSchema& event) {
    // Clear previous document
    jsonDoc.clear();
    
    // Parse JSON
    DeserializationError error = deserializeJson(jsonDoc, response);
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Check if response has success and data fields
    if (!jsonDoc["success"].is<bool>() || !jsonDoc["success"].as<bool>()) {
        Serial.println("API returned success=false");
        return false;
    }
    
    if (!jsonDoc["data"].is<JsonObject>()) {
        Serial.println("Missing 'data' object in response");
        return false;
    }
    
    JsonObject data = jsonDoc["data"].as<JsonObject>();
    
    // Extract required fields
    if (!data["_id"].is<const char*>() ||
        !data["eventType"].is<const char*>() ||
        !data["label"].is<const char*>() ||
        !data["payload"].is<JsonObject>() ||
        !data["position"].is<int>()) {
        Serial.println("Missing required fields in event data");
        return false;
    }
    
    // Populate EventSchema
    event.concertId = data["_id"].as<std::string>();
    event.eventType = data["eventType"].as<std::string>();
    event.label = data["label"].as<std::string>();
    event.payload = data["payload"].as<JsonObject>();
    event.position = data["position"].as<int>();
    event.timestamp = millis();
    
    Serial.println("Event parsed successfully");
    return true;
}
