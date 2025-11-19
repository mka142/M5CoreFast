#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Loading page with animated note images
class LoadingPage {
public:
    static lv_obj_t* create();
    static void firstRender();
    static void lastRender();
    static void update_animation(lv_timer_t *timer);
    
private:
    static lv_obj_t *note_img1;
    static lv_obj_t *note_img2;
    static lv_obj_t *note_img3;
    static int animation_step;
};
