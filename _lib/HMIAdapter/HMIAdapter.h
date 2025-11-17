#pragma once
#include "M5ModuleHMI.h"
#include <Wire.h>

class HMIAdapter
{
public:
    void begin();
    int getEncoderValue();
    bool getButtonA();
    bool getButtonB();
    bool getButtonS();

private:
    M5ModuleHMI hmi_module;
};