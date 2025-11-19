#include "RotatingNote.h"

// Declare external image
LV_IMG_DECLARE(note);

// Static member initialization
RotatingNote* RotatingNote::active_instance = nullptr;

// Rotation increments: EXACT COPY from working BeforeConcertPage
const int RotatingNote::rotation_increments[] = {
    300,   // +30° (0.1° units)
    3300,  // +330°
    600,   // +60° (360°→420°, displays as 60°)
    3000,  // +300° (60°→360°)
    900,   // +900° (360°→450°, displays as 90°)
    2700,  // +270° (90°→360°)
    1200,  // +120° (360°→480°, displays as 120°)
    1200,  // +120° (120°→240°)
    1200   // +120° (240°→360°/0°)
};
const int RotatingNote::rotation_increments_count = 9;

RotatingNote::RotatingNote() 
    : note_img(nullptr), rotation_timer(nullptr), 
      rotation_step(0), current_rotation(0) {}

lv_obj_t* RotatingNote::create(lv_obj_t* parent, uint32_t recolor, int x_offset, int y_offset) {
    // EXACT COPY from working BeforeConcertPage note creation
    note_img = lv_image_create(parent);
    lv_image_set_src(note_img, &note);
    lv_obj_align(note_img, LV_ALIGN_TOP_MID, x_offset, y_offset);
    lv_image_set_pivot(note_img, 48, 65);  // Set pivot to center (half of 96x130)
    lv_image_set_scale(note_img, 230);  // Scale down to 90% (256 = 100%)
    
    // Apply recoloring only if specified (not default 0xFFFFFFFF)
    if (recolor != 0xFFFFFFFF) {
        lv_obj_set_style_img_recolor(note_img, lv_color_hex(recolor), 0);
        lv_obj_set_style_img_recolor_opa(note_img, LV_OPA_100, 0);
    }
    
    return note_img;
}

void RotatingNote::startRotation() {
    if (!note_img || rotation_timer) return;
    
    // Reset rotation state
    rotation_step = 0;
    current_rotation = 0;
    
    // Set this as the active instance
    active_instance = this;
    
    // Start rotation animation timer (2000ms per step - SAME as BeforeConcertPage)
    rotation_timer = lv_timer_create(update_rotation_callback, 2000, nullptr);
}

void RotatingNote::stopRotation() {
    if (rotation_timer) {
        lv_timer_del(rotation_timer);
        rotation_timer = nullptr;
    }
    if (active_instance == this) {
        active_instance = nullptr;
    }
}

void RotatingNote::cleanup() {
    stopRotation();
    note_img = nullptr;
    rotation_step = 0;
    current_rotation = 0;
}

// Static callback wrapper
void RotatingNote::update_rotation_callback(lv_timer_t *timer) {
    if (active_instance) {
        active_instance->update_rotation();
    }
}

// Callback for smooth rotation animation - EXACT COPY from BeforeConcertPage
static void set_angle(void *img, int32_t v) {
    lv_image_set_rotation((lv_obj_t *)img, v);
}

void RotatingNote::update_rotation() {
    if (!note_img) return;
    
    // EXACT COPY from working BeforeConcertPage::update_rotation
    int increment = rotation_increments[rotation_step];
    int target_rotation = current_rotation + increment;
    
    // Animate rotation smoothly using lv_image_set_rotation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, note_img);
    lv_anim_set_exec_cb(&anim, set_angle);
    lv_anim_set_values(&anim, current_rotation, target_rotation);
    lv_anim_set_time(&anim, 1200);  // 1.2s smooth animation
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
    
    // Update current rotation (LVGL handles normalization internally)
    current_rotation = target_rotation;
    
    // Move to next rotation step
    rotation_step = (rotation_step + 1) % rotation_increments_count;
}
