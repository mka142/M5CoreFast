#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Before concert page with pulsing color
class BeforeConcertPage {
public:
    static lv_obj_t* create();
    static void update_pulse(lv_timer_t *timer);
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *title_label;
    static int pulse_value;
};
