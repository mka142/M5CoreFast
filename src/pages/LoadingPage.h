#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Simple loading page with animated dots
class LoadingPage {
public:
    static lv_obj_t* create();
    static void update_animation(lv_timer_t *timer);
    
private:
    static lv_obj_t *label_dots;
    static int dot_count;
};
