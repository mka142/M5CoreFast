#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"

class OvationPage : public Page
{
public:
    OvationPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(OVATION, display, text, hmi, rgb, navigator) {}

    void firstRender() override
    {
        display.clear(TFT_BLUE);
        rgb.setColor(20, 20, 20); // Blue on ovation page
        
        // Debug: Print payload info
        Serial.println("=== OvationPage firstRender ===");
        Serial.print("Concert ID: ");
        Serial.println(getPayload().concertId.c_str());
        Serial.print("Event Type: ");
        Serial.println(getPayload().eventType.c_str());
        Serial.print("Label: ");
        Serial.println(getPayload().label.c_str());
        
        if (!getPayload().payload.isNull()) {
            Serial.println("Payload contains:");
            serializeJsonPretty(getPayload().payload, Serial);
            Serial.println();
        } else {
            Serial.println("Payload is null");
        }
    }

    void render() override
    {
        text.setFont(FontStyle::Bold, FontSize::Large);
        std::string label = "Ovation!";
        const auto& payload = getPayload().payload;
        if (!payload.isNull() && payload.containsKey("message")) {
            label = payload["message"].as<String>().c_str();
        }
        text.drawText(label.c_str(), TextAlignX::Center, TextAlignY::Center, FontStyle::Bold, FontSize::Normal);
    }

    void handleInput() override
    {
    }
};
