#include "MQTTPageBridge.h"
#include <Arduino.h>

MQTTPageBridge::MQTTPageBridge(MQTTAdapter& mqttAdapter, PageNavigator& pageNavigator)
    : mqtt(mqttAdapter), pageNavigator(pageNavigator) {
}

void MQTTPageBridge::begin() {
    // Register callbacks with the MQTT adapter
    mqtt.onEvent([this](const EventSchema& event) {
        this->onMQTTEvent(event);
    });
    
    mqtt.onConnectionChange([this](bool connected) {
        this->onMQTTConnectionChange(connected);
    });
    
    Serial.println("MQTT Page Bridge initialized");
}

void MQTTPageBridge::addPageMapping(const std::string& eventType, PageID pageId) {
    pageMap[eventType] = pageId;
    reversePageMap[pageId] = eventType;
    
    Serial.print("Added page mapping: ");
    Serial.print(eventType.c_str());
    Serial.print(" -> ");
    Serial.println(static_cast<int>(pageId));
}

void MQTTPageBridge::registerPayloadHandler(PageID pageId, PagePayloadCallback callback) {
    payloadHandlers[pageId] = callback;
    
    Serial.print("Registered payload handler for page ID: ");
    Serial.println(static_cast<int>(pageId));
}

void MQTTPageBridge::loop() {
    // The actual event processing happens in the MQTT callback
    // This method is here for consistency and future extensions
}

void MQTTPageBridge::onMQTTEvent(const EventSchema& event) {
    Serial.print("Bridge received event - Type: ");
    Serial.print(event.eventType.c_str());
    Serial.print(", Concert: ");
    Serial.print(event.concertId.c_str());
    Serial.print(", Position: ");
    Serial.println(event.position);
    
    PageID targetPage = getPageIdFromEventType(event.eventType);
    if (targetPage != static_cast<PageID>(-1)) { // Valid page ID found
        // Call payload handler if registered for this page
        auto handlerIt = payloadHandlers.find(targetPage);
        if (handlerIt != payloadHandlers.end()) {
            Serial.println("Calling payload handler for page");
            handlerIt->second(event);  // Call the registered callback
        }
        
        // Navigate to the target page
        pageNavigator.showPage(targetPage);
        logEventProcessing(event, true);
    } else {
        logEventProcessing(event, false);
    }
}

void MQTTPageBridge::onMQTTConnectionChange(bool connected) {
    if (connected) {
        Serial.println("Bridge: MQTT connected - ready to receive page change events");
    } else {
        Serial.println("Bridge: MQTT disconnected - page changes from MQTT unavailable");
    }
}

PageID MQTTPageBridge::getPageIdFromEventType(const std::string& eventType) {
    auto it = pageMap.find(eventType);
    if (it != pageMap.end()) {
        return it->second;
    }
    
    Serial.print("Unknown eventType for page mapping: ");
    Serial.println(eventType.c_str());
    return static_cast<PageID>(-1); // Invalid page ID
}

std::string MQTTPageBridge::getEventTypeFromPageId(PageID pageId) {
    auto it = reversePageMap.find(pageId);
    if (it != reversePageMap.end()) {
        return it->second;
    }
    return "unknown";
}

std::string MQTTPageBridge::getCurrentPageName() {
    PageID currentPageID = pageNavigator.getCurrentPage();
    return getEventTypeFromPageId(currentPageID);
}

void MQTTPageBridge::publishCurrentPage(const char* statusTopic) {
    if (mqtt.isConnected()) {
        std::string currentPage = getCurrentPageName();
        
        // Create a simple status JSON
        String statusJson = "{\"currentPage\":\"" + String(currentPage.c_str()) + 
                           "\",\"timestamp\":" + String(millis()) + "}";
        
        mqtt.publish(statusTopic, statusJson.c_str());
        
        Serial.print("Published current page status: ");
        Serial.println(statusJson);
    }
}

void MQTTPageBridge::logEventProcessing(const EventSchema& event, bool success) {
    if (success) {
        Serial.print("✓ Successfully processed page change event: ");
        Serial.print(event.eventType.c_str());
        Serial.print(" (Concert: ");
        Serial.print(event.concertId.c_str());
        Serial.print(", Pos: ");
        Serial.print(event.position);
        Serial.println(")");
    } else {
        Serial.print("✗ Failed to process page change event: ");
        Serial.print(event.eventType.c_str());
        Serial.print(" - no mapping found (Concert: ");
        Serial.print(event.concertId.c_str());
        Serial.print(", Pos: ");
        Serial.print(event.position);
        Serial.println(")");
    }
}
