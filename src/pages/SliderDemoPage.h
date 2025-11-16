#pragma once
#include <Page.h>
#include "../../lib/ui/PageID.h"
#include "../../lib/ui/ThemeColors.h"
#include <TextRegion.h>

class SliderDemoPage : public Page
{
public:
    SliderDemoPage(DisplayAdapter &display, TextAdapter &text, HMIAdapter &hmi, RGBAdapter &rgb, IPageNavigator *navigator = nullptr)
        : Page(SLIDER_DEMO, display, text, hmi, rgb, navigator),
          textRegion(display.getDisplay(), 0, 100, 320, 80, THEME_DARK.background) {}

    void firstRender() override
    {
        display.clear(THEME_DARK.background);
        rgb.setColor(0, 0, 0);
    }

    void render() override
    {
        //textRegion.clear();
        textRegion.drawText(
            "test suwaka",
            text,
            FontStyle::Bold,
            FontSize::Large,
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
    TextRegion textRegion;
};