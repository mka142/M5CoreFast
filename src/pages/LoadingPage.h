#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>

class LoadingPage : public Page
{
public:
    LoadingPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(LOADING, display, text, hmi, rgb, navigator),
          textRegion(display.getDisplay(), 0, 100, 320, 80, THEME_NORMAL.text),
          textAdapter(text),
          lastUpdate(0),
          dotCount(0) {}

    void firstRender() override
    {
        display.clear(THEME_NORMAL.background);
        rgb.setColor(0, 100, 255); // Blue for loading
        lastUpdate = millis();
        dotCount = 0;
    }

    void render() override
    {
        unsigned long currentTime = millis();

        // Update dots every second
        if (currentTime - lastUpdate >= 1000)
        {
            lastUpdate = currentTime;
            dotCount = (dotCount + 1) % 4; // Cycle through 0, 1, 2, 3
        }

        // Clear the text region
        textRegion.clear();

        // Build the loading text with dots
        std::string loadingText = "Ładowanie";
        for (int i = 0; i < dotCount; i++)
        {
            loadingText += " .";
        }

        // Draw the loading text centered
        textRegion.drawText(
            loadingText,
            textAdapter,
            FontStyle::Bold,
            FontSize::Large,
            TextAlignX::Center,
            TextAlignY::Center,
            THEME_NORMAL.text);

        // Optional: Add a subtitle
        std::string subtitle = "Proszę czekać...";
        text.drawText(subtitle.c_str(), TextAlignX::Center, TextAlignY::Bottom,
                      FontStyle::Italic, FontSize::Small);
    }

    void handleInput() override
    {
        // Optional: Allow manual progression with encoder or button
        int encoderValue = hmi.getEncoderValue();
        if (encoderValue > 50)
        {
            // Could transition to another page after some encoder movement
            // requestPageChange(PAGE_HOME);
        }

        // Button A to skip loading (for testing)
        if (hmi.getButtonA())
        {
            requestPageChange(BEFORE_CONCERT); // Use the first page in the enum
        }
    }

    void lastRender() override
    {
        // Optional cleanup when leaving the page
        rgb.setColor(0, 0, 0); // Turn off RGB
    }

private:
    TextRegion textRegion;
    TextAdapter &textAdapter;
    unsigned long lastUpdate;
    int dotCount; // 0 = no dots, 1 = one dot, 2 = two dots, 3 = three dots
};
