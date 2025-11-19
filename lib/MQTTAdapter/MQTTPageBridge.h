#pragma once
#include "MQTTAdapter.h"
#include <PageNavigator.h>
#include <PageID.h>
#include <map>
#include <string>
#include <functional>

// Callback type for setting payload on a page
typedef std::function<void(const EventSchema&)> PagePayloadCallback;

/**
 * MQTTPageBridge - Bridge between MQTT events and LVGL page navigation
 * 
 * Listens to MQTT events and automatically navigates to corresponding pages.
 * Works with the current LVGL PageNavigator system.
 * 
 * Usage:
 *   MQTTPageBridge bridge(mqttAdapter, pageNavigator);
 *   bridge.begin();
 *   bridge.addPageMapping("BEFORE_CONCERT", BEFORE_CONCERT);
 *   bridge.registerPayloadHandler(OVATION, OvationPage::setPayload);
 *   // In loop:
 *   bridge.loop();
 */
class MQTTPageBridge {
public:
    MQTTPageBridge(MQTTAdapter& mqttAdapter, PageNavigator& pageNavigator);
    
    // Initialize the bridge and register MQTT callbacks
    void begin();
    
    // Add mapping from MQTT event type to PageID
    void addPageMapping(const std::string& eventType, PageID pageId);
    
    // Register a payload handler for a specific page
    void registerPayloadHandler(PageID pageId, PagePayloadCallback callback);
    
    // Process pending events (call in main loop)
    void loop();
    
    // Optional: Publish current page status to MQTT
    void publishCurrentPage(const char* statusTopic);
    
    // Get current page name (event type string)
    std::string getCurrentPageName();
    
private:
    MQTTAdapter& mqtt;
    PageNavigator& pageNavigator;
    
    // Bidirectional mapping between event types and page IDs
    std::map<std::string, PageID> pageMap;        // eventType -> PageID
    std::map<PageID, std::string> reversePageMap; // PageID -> eventType
    
    // Payload handlers for pages that need event data
    std::map<PageID, PagePayloadCallback> payloadHandlers;
    
    // MQTT event handlers
    void onMQTTEvent(const EventSchema& event);
    void onMQTTConnectionChange(bool connected);
    
    // Helper methods
    PageID getPageIdFromEventType(const std::string& eventType);
    std::string getEventTypeFromPageId(PageID pageId);
    void logEventProcessing(const EventSchema& event, bool success);
};
