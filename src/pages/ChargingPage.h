#pragma once
#include <lvgl.h>

// Charging page - displayed automatically when device is charging
// Can be dismissed with a click to continue normal operation
class ChargingPage {
public:
    static lv_obj_t* create();
    static void cleanup();
    static void update_battery_status();  // Call periodically to update display
    
    // Click callback to dismiss charging screen
    static void on_screen_clicked(lv_event_t *e);
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *battery_label;
    static lv_obj_t *percentage_label;
    
    static const char* get_battery_symbol(int percentage);
};
