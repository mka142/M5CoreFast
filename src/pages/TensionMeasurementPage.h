#pragma once
#include <Page.h>
#include <HTTPAdapter.h>
#include <RTCAdapter.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>
#include <vector>
#include <ArduinoJson.h>

struct TensionRecord
{
    unsigned long long t; // timestamp in milliseconds (Unix epoch)
    int v;                // value from HMI encoder
};

class TensionMeasurementPage : public Page
{
public:
    TensionMeasurementPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi,
                           RGBAdapter &rgb, RTCAdapter &rtc, HTTPAdapter &http, const char *apiHost,
                           const char *clientId, IPageNavigator *navigator = nullptr)
        : Page(TENSION_MEASUREMENT, display, text, hmi, rgb, navigator),
          rtcAdapter(rtc),
          httpAdapter(http),
          apiHost(apiHost),
          clientId(clientId),
          valueRegion(display.getDisplay(), 0, 80, 320, 100, COLOR_BLACK),
          statusRegion(display.getDisplay(), 0, 200, 320, 40, COLOR_BLACK),
          textAdapter(text),
          lastEncoderValue(0),
          currentValue(0),
          bufferSize(0),
          maxBufferSize(200)
    {
        records.reserve(maxBufferSize);
    }

    void firstRender() override
    {
        display.clear(THEME_NORMAL.background);
        rgb.setColor(255, 255, 0); // Yellow for measurement mode

        lastEncoderValue = hmi.getEncoderValue();
        currentValue = lastEncoderValue;

        Serial.println("=== TensionMeasurement page activated ===");
        Serial.print("Concert ID: ");
        Serial.println(getPayload().concertId.c_str());
        Serial.print("Event Type: ");
        Serial.println(getPayload().eventType.c_str());
        
        if (!getPayload().payload.isNull()) {
            Serial.println("Payload contains:");
            serializeJsonPretty(getPayload().payload, Serial);
            Serial.println();
            
            if (getPayload().payload.containsKey("pieceId")) {
                Serial.print("Piece ID: ");
                Serial.println(getPayload().payload["pieceId"].as<String>());
            } else {
                Serial.println("Warning: No pieceId in payload!");
            }
        } else {
            Serial.println("Warning: Payload is null!");
        }
    }

    void render() override
    {
        // Clear regions
        // valueRegion.clear();
        // statusRegion.clear();

        // Display current HMI encoder value (large, centered)
        char valueText[32];
        snprintf(valueText, sizeof(valueText), "%d", currentValue);

        valueRegion.drawText(
            std::string(valueText),
            textAdapter,
            FontStyle::Bold,
            FontSize::Large,
            TextAlignX::Center,
            TextAlignY::Center,
            COLOR_PRIMARY);

        // Display buffer status
        char statusText[64];
        snprintf(statusText, sizeof(statusText), "Buffer: %d/%d", bufferSize, maxBufferSize);

        statusRegion.drawText(
            std::string(statusText),
            textAdapter,
            FontStyle::Normal,
            FontSize::Small,
            TextAlignX::Center,
            TextAlignY::Center,
            COLOR_SECONDARY);

        // Show if sending
        if (httpAdapter.isBusy())
        {
            text.drawText("Sending...", TextAlignX::Right, TextAlignY::Bottom,
                          FontStyle::Italic, FontSize::Small);
        }
    }

    void handleInput() override
    {
        int encoderValue = hmi.getEncoderValue();

        // Check if encoder value changed
        if (encoderValue != lastEncoderValue)
        {
            lastEncoderValue = encoderValue;
            currentValue = encoderValue;

            // Record the change with Unix timestamp
            TensionRecord record;
            record.t = rtcAdapter.getUnixTimestampMs();
            record.v = encoderValue;

            records.push_back(record);
            bufferSize++;

            Serial.print("Recorded: t=");
            Serial.print((unsigned long)record.t);
            Serial.print(", v=");
            Serial.println(record.v);

            // Check if buffer is full
            if (bufferSize >= maxBufferSize)
            {
                sendBufferedData();
            }
        }

        // // Manual send with Button A
        // if (hmi.getButtonA() && bufferSize > 0)
        // {
        //     Serial.println("Manual send triggered");
        //     sendBufferedData();
        // }

        // // Clear buffer with Button B
        // if (hmi.getButtonB() && bufferSize > 0)
        // {
        //     Serial.println("Buffer cleared manually");
        //     clearBuffer();
        // }
    }

    void lastRender() override
    {
        // Send any remaining data when leaving the page
        if (bufferSize > 0)
        {
            Serial.println("Sending remaining data before page change");
            sendBufferedData();
        }
        rgb.setColor(0, 0, 0);
    }

private:
    RTCAdapter &rtcAdapter;
    HTTPAdapter &httpAdapter;
    const char *apiHost;
    const char *clientId;
    TextRegion valueRegion;
    TextRegion statusRegion;
    TextAdapter &textAdapter;

    int lastEncoderValue;
    int currentValue;
    std::vector<TensionRecord> records;
    size_t bufferSize;
    const size_t maxBufferSize;

    void sendBufferedData()
    {
        if (bufferSize == 0)
        {
            Serial.println("No data to send");
            return;
        }

        // Get pieceId from payload
        std::string pieceId = "unknown";
        if (!getPayload().payload.isNull() && getPayload().payload.containsKey("pieceId"))
        {
            pieceId = getPayload().payload["pieceId"].as<String>().c_str();
        }

        // Build JSON payload
        StaticJsonDocument<4096> doc; // Large enough for 200 records
        doc["clientId"] = clientId;
        doc["pieceId"] = pieceId.c_str();

        JsonArray dataArray = doc.createNestedArray("data");
        for (size_t i = 0; i < bufferSize; i++)
        {
            JsonObject record = dataArray.createNestedObject();
            record["t"] = records[i].t;
            record["v"] = records[i].v;
        }

        // Serialize to string
        String jsonString;
        serializeJson(doc, jsonString);

        Serial.print("Sending ");
        Serial.print(bufferSize);
        Serial.println(" records to API");

        // Build full URL
        String url = String(apiHost);

        // Send HTTP POST (non-blocking)
        httpAdapter.postJSON(url.c_str(), jsonString.c_str(),
                             [this](const HTTPResponse &response)
                             {
                                 if (response.success)
                                 {
                                     Serial.print("Data sent successfully: HTTP ");
                                     Serial.println(response.statusCode);
                                 }
                                 else
                                 {
                                     Serial.print("Failed to send data: HTTP ");
                                     Serial.println(response.statusCode);
                                 }
                             });

        // Clear buffer after queuing the request
        clearBuffer();
    }

    void clearBuffer()
    {
        records.clear();
        bufferSize = 0;
        Serial.println("Buffer cleared");
    }
};