#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <functional>
#include <queue>

struct HTTPRequest {
    std::string url;
    std::string payload;
    unsigned long timestamp;
};

struct HTTPResponse {
    int statusCode;
    std::string body;
    bool success;
};

typedef std::function<void(const HTTPResponse&)> HTTPResponseCallback;

class HTTPAdapter {
public:
    HTTPAdapter();
    void begin();
    void loop(); // Non-blocking processing
    
    // Post JSON data asynchronously
    void postJSON(const char* url, const char* jsonPayload, HTTPResponseCallback callback = nullptr);
    
    // Check if adapter is busy
    bool isBusy();
    
    // Get queue size
    size_t getQueueSize();
    
private:
    HTTPClient httpClient;
    std::queue<HTTPRequest> requestQueue;
    HTTPResponseCallback currentCallback;
    bool processingRequest;
    unsigned long requestStartTime;
    const unsigned long requestTimeout = 10000; // 10 seconds timeout
    
    void processNextRequest();
    void handleResponse(const HTTPResponse& response);
};