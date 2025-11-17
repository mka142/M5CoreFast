#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Tension measurement page - displays encoder value
// Will be enhanced later with actual encoder input and data recording
class TensionMeasurementPage {
public:
    static lv_obj_t* create();
    static void updateValue(int value);
    static void updateBuffer(int count, int max);
    
private:
    static lv_obj_t *value_label;
    static lv_obj_t *buffer_label;
};
