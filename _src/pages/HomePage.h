
#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>

class HomePage : public Page
{
public:
    HomePage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(PAGE_HOME, display, text, hmi, rgb, navigator),
          textRegion(display.getDisplay(), 0, 20, 300, 60, THEME_NORMAL.error),
          textAdapter(text) {}

    void firstRender() override
    {
        // display wh
        Serial.println(display.width());
        Serial.println(display.height());
        display.clear();
        rgb.setColor(0, 255, 0); // Green on home page
        // int fontHeight = display.getDisplay().fontHeight();
        //  textRegion.setHeight(fontHeight * 2); // Example: double font height
        //   textRegion.drawText(
        //       "Welcome Home!",
        //       textAdapter,
        //       FontStyle::Bold,
        //       FontSize::Large,
        //       TextAlignX::Center,
        //       TextAlignY::Top,
        //       TFT_WHITE
        //   );
    }

    void render() override
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "Encoder: %d", encoderValue);
        // int fontHeight = display.getDisplay().fontHeight();
        textRegion.drawText(
            std::string(buf),
            textAdapter,
            FontStyle::Normal,
            FontSize::Normal,
            TextAlignX::Left,
            TextAlignY::Bottom,
            THEME_NORMAL.background); // Use theme color
    }

    void handleInput() override
    {
        encoderValue = hmi.getEncoderValue();
        if (encoderValue > 100)
        {
            requestPageChange(PAGE_INFO);
        }
    }

private:
    int encoderValue = 0;
    TextRegion textRegion;
    TextAdapter &textAdapter;
};
