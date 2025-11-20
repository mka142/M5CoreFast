#include "HTTPAdapter.h"
#include <Arduino.h>

HTTPAdapter::HTTPAdapter() 
    : processingRequest(false), requestStartTime(0), currentCallback(nullptr) {
}

void HTTPAdapter::begin() {
    Serial.println("HTTP Adapter initialized");
}

void HTTPAdapter::loop() {
    if (!processingRequest && !requestQueue.empty()) {
        processNextRequest();
    }
    
    // Check for timeout if processing
    if (processingRequest) {
        unsigned long now = millis();
        if (now - requestStartTime > requestTimeout) {
            Serial.println("HTTP request timeout");
            HTTPResponse response = {0, "", false};
            handleResponse(response);
            processingRequest = false;
            httpClient.end();
        }
    }
}

void HTTPAdapter::postJSON(const char* url, const char* jsonPayload, HTTPResponseCallback callback) {
    HTTPRequest request = {
        std::string(url),
        std::string(jsonPayload),
        millis()
    };
    
    requestQueue.push(request);
    
    if (callback) {
        currentCallback = callback;
    }
    
    Serial.print("Queued HTTP POST to: ");
    Serial.println(url);
    Serial.print("Queue size: ");
    Serial.println(requestQueue.size());
}

bool HTTPAdapter::isBusy() {
    return processingRequest || !requestQueue.empty();
}

size_t HTTPAdapter::getQueueSize() {
    return requestQueue.size();
}

void HTTPAdapter::processNextRequest() {
    if (requestQueue.empty()) {
        return;
    }
    
    HTTPRequest request = requestQueue.front();
    requestQueue.pop();
    
    processingRequest = true;
    requestStartTime = millis();
    
    Serial.print("Processing HTTP POST to: ");
    Serial.println(request.url.c_str());
    
    httpClient.begin(request.url.c_str());
    httpClient.addHeader("Content-Type", "application/json");
    
    int httpCode = httpClient.POST(request.payload.c_str());
    
    HTTPResponse response;
    response.statusCode = httpCode;
    response.success = (httpCode >= 200 && httpCode < 300);
    
    if (response.success) {
        response.body = httpClient.getString().c_str();
        Serial.print("HTTP POST success: ");
        Serial.println(httpCode);
    } else {
        Serial.print("HTTP POST failed: ");
        Serial.println(httpCode);
    }
    
    httpClient.end();
    processingRequest = false;
    
    handleResponse(response);
}

void HTTPAdapter::handleResponse(const HTTPResponse& response) {
    if (currentCallback) {
        currentCallback(response);
        currentCallback = nullptr; // Clear callback after use
    }
}

// Synchronous GET request
int HTTPAdapter::get(const char* url) {
    Serial.print("HTTP GET: ");
    Serial.println(url);
    
    HTTPClient client;
    client.begin(url);
    
    int httpCode = client.GET();
    
    Serial.print("HTTP GET response code: ");
    Serial.println(httpCode);
    
    client.end();
    return httpCode;
}

// Synchronous POST request
int HTTPAdapter::post(const char* url, const String& payload, const char* contentType) {
    Serial.print("HTTP POST: ");
    Serial.println(url);
    Serial.print("Payload: ");
    Serial.println(payload);
    
    HTTPClient client;
    client.begin(url);
    client.addHeader("Content-Type", contentType);
    
    int httpCode = client.POST(payload);
    
    Serial.print("HTTP POST response code: ");
    Serial.println(httpCode);
    
    client.end();
    return httpCode;
}