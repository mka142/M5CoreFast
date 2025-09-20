
#pragma once
#include <M5GFX.h>
#include <string>
#include "../display/TextAdapter.h"

class TextRegion
{
public:
    TextRegion(M5GFX &display, int x, int y, int w, int h, uint32_t bgColor)
        : display(&display), canvas(&display), xpos(x), ypos(y), width(w), height(h), bgColor(bgColor), lastText("")
    {
        canvas.setColorDepth(8);
        canvas.createSprite(width, height);
        canvas.fillSprite(bgColor);
    }

    // Draw text with alignment using TextAdapter font selection
    void drawText(const std::string &text, TextAdapter &adapter, FontStyle style, FontSize size,
                  TextAlignX alignX, TextAlignY alignY, uint32_t textColor = TFT_BLACK)
    {
        const GFXfont *font = adapter.getFont(style, size);
        canvas.setFont(font);
        int textW = canvas.textWidth(text.c_str());
        int textH = canvas.fontHeight();
        int x = 0, y = 0;
        switch (alignX)
        {
        case TextAlignX::Left:
            x = 0;
            break;
        case TextAlignX::Center:
            x = (width - textW) / 2;
            break;
        case TextAlignX::Right:
            x = width - textW;
            break;
        }
        switch (alignY)
        {
        case TextAlignY::Top:
            y = 0;
            break;
        case TextAlignY::Center:
            y = (height - textH) / 2;
            break;
        case TextAlignY::Bottom:
            y = height - textH;
            break;
        }
        if (text == lastText)
            return;
        lastText = text;
        canvas.fillSprite(bgColor);
        canvas.setTextColor(textColor);
        canvas.setCursor(x, y);
        canvas.println(text.c_str());
        canvas.pushSprite(xpos, ypos);
    }

    // Draw text using TextAdapter font selection (no alignment)
    void drawText(const std::string &text, TextAdapter &adapter, FontStyle style, FontSize size, uint32_t textColor = TFT_BLACK)
    {
        const GFXfont *font = adapter.getFont(style, size);
        drawText(text, font, textColor);
    }

    void setFont(const GFXfont *font)
    {
        canvas.setFont(font);
    }

    // Set the height of the text region and recreate the sprite
    void setHeight(int newHeight)
    {
        height = newHeight;
        canvas.deleteSprite();
        canvas.createSprite(width, height);
        canvas.fillSprite(bgColor);
    }

    void drawText(const std::string &text, const GFXfont *font, uint32_t textColor = TFT_BLACK)
    {
        if (text == lastText)
            return; // Only redraw if text changed
        lastText = text;
        canvas.fillSprite(bgColor);
        canvas.setFont(font);
        canvas.setTextColor(textColor);
        canvas.setCursor(0, 0);
        canvas.println(text.c_str());
        canvas.pushSprite(xpos, ypos);
    }

    void clear()
    {
        canvas.fillSprite(bgColor);
        canvas.pushSprite(xpos, ypos);
        lastText = "";
    }

private:
    M5GFX *display;
    M5Canvas canvas;
    int xpos, ypos, width, height;
    uint32_t bgColor;
    std::string lastText;
};
