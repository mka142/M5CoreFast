#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <RotatingNote.h>

class FormFinishedPage {
public:
    static lv_obj_t* create();
    static void cleanup();
    
private:
    static lv_obj_t* screen;
    static lv_obj_t* main_label;
    static lv_obj_t* subtitle_label;
    static RotatingNote* rotating_note;
};
