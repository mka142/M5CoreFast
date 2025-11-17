#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Slider demo page - shows how to use LVGL slider widget
class SliderDemoPage {
public:
    static lv_obj_t* create();
    static void slider_event_cb(lv_event_t *e);
    
private:
    static lv_obj_t *value_label;
};
