#pragma once
#include "DisplayAdapter.h"

// Font style and size enums
enum class FontStyle
{
    Normal,
    Bold,
    Italic,
    Serif,
};
enum class FontSize
{
    Small,
    Normal,
    Medium,
    Large
};
enum class TextAlignX
{
    Left,
    Center,
    Right
};
enum class TextAlignY
{
    Top,
    Center,
    Bottom
};

// Font configuration struct
struct FontConfig
{
    const GFXfont *small;
    const GFXfont *normal;
    const GFXfont *medium;
    const GFXfont *large;
};

class TextAdapter
{
public:
    void begin(DisplayAdapter *disp);
    void configureFonts();
    void setFont(FontStyle style, FontSize size);
    void drawText(const char *text, int x, int y, FontStyle style, FontSize size);
    void drawText(const char *text, TextAlignX alignX, TextAlignY alignY, FontStyle style, FontSize size);
    int measureTextWidth(const char *text, FontStyle style, FontSize size);
    int measureTextHeight(const char *text, FontStyle style, FontSize size);
    const GFXfont *getFont(FontStyle style, FontSize size);

private:
    DisplayAdapter *display;
    FontConfig normalFonts;
    FontConfig boldFonts;
    FontConfig italicFonts;
    FontConfig serifFonts;
    int getTextWidth(const char *text);
    int getTextHeight();
};