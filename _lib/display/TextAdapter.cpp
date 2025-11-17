#include "TextAdapter.h"
#include <M5GFX.h>
// ...existing includes...

TextAdapter::TextAdapter(DisplayAdapter &display)
    : display(display)
{
    // Optionally, initialize fonts or other members here
}
#include "TextAdapter.h"
#include <M5GFX.h>
#include "../ui/fonts/FreeSans9pt8b.h"
#include "../ui/fonts/FreeSans12pt8b.h"
#include "../ui/fonts/FreeSans18pt8b.h"
#include "../ui/fonts/FreeSans24pt8b.h"
#include "../ui/fonts/FreeSansBold9pt8b.h"
#include "../ui/fonts/FreeSansBold12pt8b.h"
#include "../ui/fonts/FreeSansBold18pt8b.h"
#include "../ui/fonts/FreeSansBold24pt8b.h"
#include "../ui/fonts/FreeSansOblique9pt8b.h"
#include "../ui/fonts/FreeSansOblique12pt8b.h"
#include "../ui/fonts/FreeSansOblique18pt8b.h"
#include "../ui/fonts/FreeSansOblique24pt8b.h"
#include "../ui/fonts/FreeSerif9pt8b.h"
#include "../ui/fonts/FreeSerif12pt8b.h"
#include "../ui/fonts/FreeSerif18pt8b.h"
#include "../ui/fonts/FreeSerif24pt8b.h"

void TextAdapter::begin(DisplayAdapter &disp)
{
    display = disp;
    configureFonts();
    Serial.println("Text adapter initialized.");
}

void TextAdapter::configureFonts()
{
    // Normal fonts
    normalFonts.small = &FreeSans9pt8b;
    normalFonts.normal = &FreeSans12pt8b;
    normalFonts.medium = &FreeSans18pt8b;
    normalFonts.large = &FreeSans24pt8b;
    // Bold fonts
    boldFonts.small = &FreeSansBold9pt8b;
    boldFonts.normal = &FreeSansBold12pt8b;
    boldFonts.medium = &FreeSansBold18pt8b;
    boldFonts.large = &FreeSansBold24pt8b;
    // Italic fonts
    italicFonts.small = &FreeSansOblique9pt8b;
    italicFonts.normal = &FreeSansOblique12pt8b;
    italicFonts.medium = &FreeSansOblique18pt8b;
    italicFonts.large = &FreeSansOblique24pt8b;

    // Serif fonts
    serifFonts.small = &FreeSerif9pt8b;
    serifFonts.normal = &FreeSerif12pt8b;
    serifFonts.medium = &FreeSerif18pt8b;
    serifFonts.large = &FreeSerif24pt8b;
}

const GFXfont *TextAdapter::getFont(FontStyle style, FontSize size)
{
    switch (style)
    {
    case FontStyle::Normal:
        switch (size)
        {
        case FontSize::Small:
            return normalFonts.small;
        case FontSize::Normal:
            return normalFonts.normal;
        case FontSize::Medium:
            return normalFonts.medium;
        case FontSize::Large:
            return normalFonts.large;
        }
        break;
    case FontStyle::Bold:
        switch (size)
        {
        case FontSize::Small:
            return boldFonts.small;
        case FontSize::Normal:
            return boldFonts.normal;
        case FontSize::Medium:
            return boldFonts.medium;
        case FontSize::Large:
            return boldFonts.large;
        }
        break;
    case FontStyle::Italic:
        switch (size)
        {
        case FontSize::Small:
            return italicFonts.small;
        case FontSize::Normal:
            return italicFonts.normal;
        case FontSize::Medium:
            return italicFonts.medium;
        case FontSize::Large:
            return italicFonts.large;
        }
        break;
    case FontStyle::Serif:
        switch (size)
        {
        case FontSize::Small:
            return serifFonts.small;
        case FontSize::Normal:
            return serifFonts.normal;
        case FontSize::Medium:
            return serifFonts.medium;
        case FontSize::Large:
            return serifFonts.large;
        }
        break;
    }
    return normalFonts.normal;
}

void TextAdapter::setFont(FontStyle style, FontSize size)
{
    display.getDisplay().setFont(getFont(style, size));
}

int TextAdapter::getTextWidth(const std::string &text)
{
    return display.getDisplay().textWidth(text.c_str());
}

int TextAdapter::getTextHeight()
{
    return display.getDisplay().fontHeight();
}

int TextAdapter::measureTextWidth(const std::string &text, FontStyle style, FontSize size)
{
    setFont(style, size);
    return getTextWidth(text);
}

int TextAdapter::measureTextHeight(const std::string &text, FontStyle style, FontSize size)
{
    setFont(style, size);
    return getTextHeight();
}

void TextAdapter::drawText(const std::string &text, int x, int y, FontStyle style, FontSize size)
{
    setFont(style, size);
    display.getDisplay().setCursor(x, y);
    display.getDisplay().print(text.c_str());
}

void TextAdapter::drawText(const std::string &text, TextAlignX alignX, TextAlignY alignY, FontStyle style, FontSize size)
{
    setFont(style, size);
    int w = display.width();
    int h = display.height();
    int textW = getTextWidth(text);
    int textH = getTextHeight();

    int x = 0, y = 0;
    switch (alignX)
    {
    case TextAlignX::Left:
        x = 0;
        break;
    case TextAlignX::Center:
        x = (w - textW) / 2;
        break;
    case TextAlignX::Right:
        x = w - textW;
        break;
    }
    switch (alignY)
    {
    case TextAlignY::Top:
        y = 0;
        break;
    case TextAlignY::Center:
        y = (h - textH) / 2;
        break;
    case TextAlignY::Bottom:
        y = h - textH;
        break;
    }
    drawText(text, x, y, style, size);
}