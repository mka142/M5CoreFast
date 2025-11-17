#pragma once
#include <MQTTAdapter.h>
#include <PageManager.h>
#include <map>
#include <string>

class MQTTPageBridge {
public:
    MQTTPageBridge(MQTTAdapter& mqttAdapter, PageManager& pageManager);
    void begin();
    void addPageMapping(const std::string& eventType, PageID pageId);
    void loop(); // Optional: for additional processing
    
    // Status and diagnostics
    std::string getCurrentPageName();
    void publishCurrentPage(const char* statusTopic);
    
private:
    MQTTAdapter& mqtt;
    PageManager& pageManager;
    std::map<std::string, PageID> pageMap;
    std::map<PageID, std::string> reversePageMap;
    
    // Event handlers
    void onMQTTEvent(const EventSchema& event);
    void onMQTTConnectionChange(bool connected);
    
    // Helper methods
    PageID getPageIdFromEventType(const std::string& eventType);
    std::string getEventTypeFromPageId(PageID pageId);
    void logEventProcessing(const EventSchema& event, bool success);
};