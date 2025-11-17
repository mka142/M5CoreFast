#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>

class BeforeConcertPage : public Page
{
public:
    BeforeConcertPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(BEFORE_CONCERT, display, text, hmi, rgb, navigator), pulseStartTime(0),
          pulsePeriod(2000)
    {
    } // 2 second pulse cycle

    void firstRender() override
    {
        display.clear(THEME_DARK.background);
        pulseStartTime = millis();
    }

    void render() override
    {
        unsigned long currentTime = millis();

        // Calculate pulse intensity (0-255) using sine wave
        float pulsePhase = (float)(currentTime - pulseStartTime) / pulsePeriod * 2.0 * PI;
        float pulseIntensity = (sin(pulsePhase) + 1.0) / 2.0; // Normalize to 0-1
        int blueValue = (int)(pulseIntensity * 255);          // Scale to 0-255

        // Set pulsating blue RGB
        rgb.setColor(0, 0, blueValue);

        // Draw the main question
        text.drawText("Co czują Wrocławianie?",
                      TextAlignX::Center,
                      TextAlignY::Top,
                      FontStyle::Bold,
                      FontSize::Large);

        // Optional: Add a subtitle or instruction
        text.drawText("Przygotowanie do koncertu...",
                      TextAlignX::Center, TextAlignY::Bottom,
                      FontStyle::Italic, FontSize::Small);
    }

    void handleInput() override
    {
        // Handle encoder input for navigation
        int encoderValue = hmi.getEncoderValue();

        // Button A to advance to next page
        if (hmi.getButtonA())
        {
            requestPageChange(APP_GUIDE); // Next page in sequence
        }

        // Button B to go back to loading
        if (hmi.getButtonB())
        {
            requestPageChange(LOADING);
        }

        // Encoder rotation for different intensity/speed
        if (encoderValue > 100)
        {
            pulsePeriod = 1000; // Faster pulse
        }
        else if (encoderValue < -100)
        {
            pulsePeriod = 3000; // Slower pulse
        }
        else
        {
            pulsePeriod = 2000; // Normal pulse
        }
    }

    void lastRender() override
    {
        // Turn off RGB when leaving the page
        rgb.setColor(0, 0, 0);
    }

private:
    unsigned long pulseStartTime;
    unsigned long pulsePeriod; // Pulse cycle duration in milliseconds
};
