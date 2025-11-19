#pragma once
#include <Arduino.h>
#include <lvgl.h>

class RotatingNote {
public:
    RotatingNote();
    
    // Create and configure the rotating note
    // Pass parent screen and optional color for recoloring (0xFFFFFFFF = no recolor, use original)
    lv_obj_t* create(lv_obj_t* parent, uint32_t recolor = 0xFFFFFFFF, int x_offset = 0, int y_offset = 10);
    
    // Start/stop rotation animation
    void startRotation();
    void stopRotation();
    
    // Cleanup
    void cleanup();
    
private:
    lv_obj_t* note_img;
    lv_timer_t* rotation_timer;
    int rotation_step;
    int current_rotation;
    
    // Static instance pointer for timer callback (single instance support)
    static RotatingNote* active_instance;
    
    static void update_rotation_callback(lv_timer_t *timer);
    void update_rotation();
    
    // Rotation increments (copied from working BeforeConcertPage)
    static const int rotation_increments[];
    static const int rotation_increments_count;
};
