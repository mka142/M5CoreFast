#pragma once
#include <lvgl.h>

/**
 * @brief Sponsors Page - displays rotating sponsor logos
 * Shows at beginning and end of concert
 */
class SponsorsPage {
public:
    static lv_obj_t* create();
    static void cleanup();
    
private:
    static lv_obj_t *screen;
};
