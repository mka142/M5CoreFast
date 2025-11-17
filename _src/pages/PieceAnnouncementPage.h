#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>
#include <vector>
#include <ArduinoJson.h>

class PieceAnnouncementPage : public Page
{
public:
    PieceAnnouncementPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(PIECE_ANNOUNCEMENT, display, text, hmi, rgb, navigator),
          composerRegion(display.getDisplay(), 0, 80, 320, 60, THEME_DARK.background),
          pieceRegion(display.getDisplay(), 0, 150, 320, 60, THEME_DARK.background) {}

    void firstRender() override
    {
        display.clear(THEME_DARK.background);
        rgb.setColor(0, 0, 0);
        
        // Debug: Print payload info
        Serial.println("=== PieceAnnouncementPage firstRender ===");
        if (!getPayload().payload.isNull()) {
            Serial.println("Payload contains:");
            serializeJsonPretty(getPayload().payload, Serial);
            Serial.println();
        } else {
            Serial.println("Warning: Payload is null");
        }
    }

    void render() override
    {
        //composerRegion.clear();
        //pieceRegion.clear();
        
        std::string composer = "Kompozytor nieznany";
        std::string piece = "Utwór nieznany";
        
        const auto& payload = getPayload().payload;
        if (!payload.isNull()) {
            if (payload.containsKey("composerName")) {
                composer = payload["composerName"].as<String>().c_str();
            }
            if (payload.containsKey("pieceTitle")) {
                piece = payload["pieceTitle"].as<String>().c_str();
            }
        }
        
        // Display composer name
        composerRegion.drawText(
            composer,
            text,
            FontStyle::Bold,
            FontSize::Normal,
            TextAlignX::Center,
            TextAlignY::Center,
            THEME_DARK.accent
        );
        
        // Display piece title
        pieceRegion.drawText(
            piece,
            text,
            FontStyle::Normal,
            FontSize::Normal,
            TextAlignX::Center,
            TextAlignY::Center,
            THEME_DARK.text
        );
    }

    void handleInput() override
    {
        // No input handling for now
    }

    void lastRender() override
    {
        rgb.setColor(0, 0, 0);
    }

private:
    TextRegion composerRegion;
    TextRegion pieceRegion;
};