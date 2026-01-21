/**
 * @file SliderDemoPage.h
 * @brief Simple intro page for slider/tension measurement demo
 */

#pragma once

#include <lvgl.h>
#include <ThemeColors.h>

class SliderDemoPage {
public:
    static lv_obj_t* create();
    static void firstRender();
    static void lastRender();
    static void cleanup();
    
    // Button callback
    static void on_test_button_clicked(lv_event_t *e);
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *button;
};
