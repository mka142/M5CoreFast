#include "RGBAdapter.h"

void RGBAdapter::begin(int pin, int num_leds)
{
    _num_leds = num_leds;
    rgb = new Adafruit_NeoPixel(num_leds, pin, NEO_GRB + NEO_KHZ800);
    rgb->begin();
    rgb->show();
    Serial.println("RGB adapter initialized.");
}

void RGBAdapter::setColor(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = rgb->Color(r, g, b);
    for (int i = 0; i < _num_leds; i++)
    {
        rgb->setPixelColor(i, color);
    }
    rgb->show();
}