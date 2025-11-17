#pragma once
#include <M5GFX.h>

class DisplayAdapter
{
public:
    void begin();
    void configure();
    void clear(uint32_t color = TFT_BLACK);
    void update();
    void waitDisplay();
    int width();
    int height();
    M5GFX &getDisplay();

private:
    M5GFX display;
};