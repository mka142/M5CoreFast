#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"

class TestPage2 : public Page
{
public:
    TestPage2(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(PAGE_INFO, display, text, hmi, rgb, navigator) {}

    void firstRender() override
    {
        display.clear(TFT_BLUE);
        rgb.setColor(0, 0, 255); // Blue on info page
    }

    void render() override
    {
        text.setFont(FontStyle::Bold, FontSize::Large);
        text.drawText("Test Page 2", TextAlignX::Center, TextAlignY::Center, FontStyle::Bold, FontSize::Large);
    }

    void handleInput() override
    {
        int encoderValue = hmi.getEncoderValue();
        if (encoderValue < 100)
        {
            requestPageChange(PAGE_HOME);
        }
    }
};
