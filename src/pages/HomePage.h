
#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"

class HomePage : public Page
{
public:
    HomePage(DisplayAdapter *display, TextAdapter *text, HMIAdapter *hmi, RGBAdapter *rgb, IPageNavigator *navigator = nullptr)
        : Page(PAGE_HOME, display, text, hmi, rgb, navigator) {}

    void firstRender() override
    {
        display->clear();
        rgb->setColor(0, 255, 0); // Green on home page
        // encoderRegion.setBackgroundColor(THEME_NORMAL.surface);
    }

    void render() override
    {
        // text->setFont(FontStyle::Bold, FontSize::Large);
        // text->drawText("Welcome Home!", TextAlignX::Center, TextAlignY::Top, FontStyle::Bold, FontSize::Large);

        // Draw encoder value
        char buf[32];
        snprintf(buf, sizeof(buf), "Encoder: %d", encoderValue);
    }

    void handleInput() override
    {
        encoderValue = hmi->getEncoderValue();
        if (encoderValue > 100)
        {
            requestPageChange(PAGE_INFO);
        }
    }

private:
    int encoderValue = 0;
};
