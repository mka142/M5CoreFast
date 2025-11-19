#pragma once
#include <Arduino.h>
#include <lvgl.h>

class EndOfConcertPage {
public:
    static lv_obj_t* create();
    static void on_form_button_clicked(lv_event_t *e);
    
private:
    static lv_obj_t* screen;
    static lv_obj_t* main_label;
    static lv_obj_t* description_label;
    static lv_obj_t* button;
};
