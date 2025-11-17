#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Before concert page with animated text, rotating note, and gradient button
class BeforeConcertPage {
public:
    static lv_obj_t* create();
    static void cleanup();
    
    // Animation callbacks
    static void update_rotation(lv_timer_t *timer);
    static void update_text(lv_timer_t *timer);
    static void text_scale_anim_cb(void *var, int32_t value);
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *note_img;
    static lv_obj_t *header_label;
    static lv_obj_t *button;
    
    static int rotation_step;
    static int current_rotation;  // Track absolute rotation position
    static int text_index;
    static bool text_scaling_out;
    static lv_anim_t text_anim;
    
    // Text array
    static const char* header_texts[];
    static const int header_texts_count;
    
    // Rotation increments (always move right/clockwise)
    static const int rotation_increments[];
    static const int rotation_increments_count;
};
