#include "MQTTPageBridge.h"
#include <Arduino.h>

MQTTPageBridge::MQTTPageBridge(MQTTAdapter& mqttAdapter, PageManager& pageManager)
    : mqtt(mqttAdapter), pageManager(pageManager) {
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

void MQTTPageBridge::loop() {
    // Optional: Add any additional processing here
    // For now, event processing happens in callbacks
}

void MQTTPageBridge::onMQTTEvent(const EventSchema& event) {
    Serial.print("Bridge received event - Type: ");
    Serial.print(event.eventType.c_str());
    Serial.print(", Concert: ");
    Serial.print(event.concertId.c_str());
    Serial.print(", Position: ");
    Serial.println(event.position);
    
    PageID targetPage = getPageIdFromEventType(event.eventType);
    if (targetPage != static_cast<PageID>(-1)) { // Assuming -1 is invalid
        // Get the target page and set its payload BEFORE switching
        Page* page = pageManager.getPage(targetPage);
        if (page) {
            page->setPayload(event);
            Serial.println("Payload set on target page");
        } else {
            Serial.println("Warning: Target page not found");
        }
        
        pageManager.requestPageChange(targetPage);
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
    Page* currentPage = pageManager.getCurrentPage();
    if (currentPage) {
        // Assuming Page has a getID() method
        return getEventTypeFromPageId(currentPage->getID());
    }
    return "none";
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