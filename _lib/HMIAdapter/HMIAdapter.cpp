#include "HMIAdapter.h"

void HMIAdapter::begin()
{
    Wire1.end();
    hmi_module.begin(&Wire1, 0x41, 12, 11, 100000);
    hmi_module.setLEDStatus(0, 1);
    hmi_module.setLEDStatus(1, 1);
    hmi_module.resetCounter();
    Serial.println("HMI adapter initialized.");
}

int HMIAdapter::getEncoderValue() { return hmi_module.getEncoderValue(); }
bool HMIAdapter::getButtonA() { return hmi_module.getButton1(); }
bool HMIAdapter::getButtonB() { return hmi_module.getButton2(); }
bool HMIAdapter::getButtonS() { return hmi_module.getButtonS(); }