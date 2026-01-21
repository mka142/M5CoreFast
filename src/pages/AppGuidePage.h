#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// App guide page with instructions for concert attendees
class AppGuidePage {
public:
    static lv_obj_t* create();
    static void firstRender();
    static void lastRender();
    static void cleanup();
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *scroll_container;
    
    // Helper function to create info cards
    static void create_info_card(lv_obj_t *parent, const char *icon, const char *title, const char *description, int y_offset);
};
