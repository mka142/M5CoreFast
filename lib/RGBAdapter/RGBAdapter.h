#pragma once
#include <Adafruit_NeoPixel.h>

class RGBAdapter
{
public:
    void begin(int pin, int num_leds);
    void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
    Adafruit_NeoPixel *rgb;
    int _num_leds;
};