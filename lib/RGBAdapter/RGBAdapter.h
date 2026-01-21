#pragma once
#include <Adafruit_NeoPixel.h>

class RGBAdapter
{
public:
    void begin(int pin, int num_leds);
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setPixel(int index, uint8_t r, uint8_t g, uint8_t b);
    void show();
    void clear();
    int getNumLeds() const { return _num_leds; }

private:
    Adafruit_NeoPixel *rgb;
    int _num_leds;
};