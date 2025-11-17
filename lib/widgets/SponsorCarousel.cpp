#include "SponsorCarousel.h"
#include <Arduino.h>
#include <algorithm>
#include <utility>  // for std::swap

// Static member initialization
lv_obj_t* SponsorCarousel::container = nullptr;
lv_obj_t* SponsorCarousel::parent_obj = nullptr;
int SponsorCarousel::pos_x = 0;
int SponsorCarousel::pos_y = 0;
int SponsorCarousel::width = 0;
int SponsorCarousel::height = 0;
std::vector<const lv_image_dsc_t*> SponsorCarousel::logos;
lv_obj_t* SponsorCarousel::img_current = nullptr;
lv_obj_t* SponsorCarousel::img_next = nullptr;
lv_timer_t* SponsorCarousel::timer = nullptr;
lv_anim_t SponsorCarousel::fade_out_anim;
lv_anim_t SponsorCarousel::fade_in_anim;
bool SponsorCarousel::random_mode = false;
uint32_t SponsorCarousel::display_duration = 3000;
std::vector<int> SponsorCarousel::batch_order;
int SponsorCarousel::batch_index = 0;
int SponsorCarousel::last_logo_index = -1;
int SponsorCarousel::current_index = -1;
bool SponsorCarousel::animating = false;

void SponsorCarousel::init(lv_obj_t *parent, int x, int y, int w, int h) {
    parent_obj = parent;
    pos_x = x;
    pos_y = y;
    width = w;
    height = h;
    
    // Create main container
    container = lv_obj_create(parent);
    lv_obj_set_pos(container, x, y);
    lv_obj_set_size(container, w, h);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    
    Serial.println("[SponsorCarousel] Initialized");
}

void SponsorCarousel::addLogo(const lv_image_dsc_t *img_desc) {
    if (img_desc) {
        logos.push_back(img_desc);
        Serial.printf("[SponsorCarousel] Added logo %d\n", logos.size());
    }
}

void SponsorCarousel::start(uint32_t duration_ms, bool random) {
    if (logos.empty()) {
        Serial.println("[SponsorCarousel] ERROR: No logos added!");
        return;
    }
    
    display_duration = duration_ms;
    random_mode = random;
    
    // Create image containers if needed
    if (!img_current) {
        img_current = lv_image_create(container);
        lv_obj_center(img_current);
        lv_obj_set_style_image_opa(img_current, LV_OPA_COVER, 0);
    }
    
    if (!img_next) {
        img_next = lv_image_create(container);
        lv_obj_center(img_next);
        lv_obj_set_style_image_opa(img_next, LV_OPA_TRANSP, 0);
    }
    
    // Create display order (one-time shuffle if random requested)
    batch_order.clear();
    for (int i = 0; i < logos.size(); i++) {
        batch_order.push_back(i);
    }
    
    if (random_mode) {
        // Fisher-Yates shuffle - done ONCE
        for (int i = batch_order.size() - 1; i > 0; i--) {
            int j = ::random(0, i + 1);  // Use global random() function
            std::swap(batch_order[i], batch_order[j]);
        }
        Serial.print("[SponsorCarousel] Shuffled order: ");
        for (int idx : batch_order) {
            Serial.printf("%d ", idx);
        }
        Serial.println();
    }
    
    // Start with first logo in order
    batch_index = 0;
    current_index = -1;
    animating = false;
    show_next_logo();
    
    // Create timer
    timer = lv_timer_create(timer_cb, display_duration, nullptr);
    
    Serial.printf("[SponsorCarousel] Started (%s, %dms/logo, %d logos)\n", 
                  random_mode ? "shuffled" : "sequential", display_duration, logos.size());
}

void SponsorCarousel::stop() {
    if (timer) {
        lv_timer_delete(timer);
        timer = nullptr;
    }
    Serial.println("[SponsorCarousel] Stopped");
}

void SponsorCarousel::cleanup() {
    stop();
    
    if (img_current) {
        lv_obj_delete(img_current);
        img_current = nullptr;
    }
    
    if (img_next) {
        lv_obj_delete(img_next);
        img_next = nullptr;
    }
    
    if (container) {
        lv_obj_delete(container);
        container = nullptr;
    }
    
    logos.clear();
    batch_order.clear();
    current_index = -1;
    batch_index = 0;
    last_logo_index = -1;
    
    Serial.println("[SponsorCarousel] Cleaned up");
}

void SponsorCarousel::timer_cb(lv_timer_t *tmr) {
    if (!animating) {
        show_next_logo();
    }
}

void SponsorCarousel::shuffle_batch() {
    // Not used anymore - shuffle is one-time in start()
}

void SponsorCarousel::show_next_logo() {
    if (logos.empty() || animating) return;
    
    animating = true;
    
    // Get next index from order (loops automatically)
    int next_index = batch_order[batch_index];
    batch_index = (batch_index + 1) % batch_order.size();  // Loop through order
    
    Serial.printf("[SponsorCarousel] Showing logo %d/%d (index %d)\n", 
                  batch_index, batch_order.size(), next_index);
    
    // Set next logo image
    lv_image_set_src(img_next, logos[next_index]);
    
    // Setup fade animations
    const uint32_t fade_duration = 500; // 500ms fade
    
    // Fade OUT current logo
    if (current_index >= 0) {
        lv_anim_init(&fade_out_anim);
        lv_anim_set_var(&fade_out_anim, img_current);
        lv_anim_set_exec_cb(&fade_out_anim, [](void *var, int32_t value) {
            lv_obj_set_style_image_opa((lv_obj_t*)var, value, 0);
        });
        lv_anim_set_values(&fade_out_anim, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_duration(&fade_out_anim, fade_duration);
        lv_anim_set_path_cb(&fade_out_anim, lv_anim_path_ease_in_out);
        lv_anim_start(&fade_out_anim);
    }
    
    // Fade IN next logo
    lv_anim_init(&fade_in_anim);
    lv_anim_set_var(&fade_in_anim, img_next);
    lv_anim_set_exec_cb(&fade_in_anim, [](void *var, int32_t value) {
        lv_obj_set_style_image_opa((lv_obj_t*)var, value, 0);
    });
    lv_anim_set_values(&fade_in_anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&fade_in_anim, fade_duration);
    lv_anim_set_path_cb(&fade_in_anim, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&fade_in_anim, [](lv_anim_t *a) {
        // Swap current and next
        lv_obj_t *temp = SponsorCarousel::img_current;
        SponsorCarousel::img_current = SponsorCarousel::img_next;
        SponsorCarousel::img_next = temp;
        SponsorCarousel::animating = false;
    });
    lv_anim_start(&fade_in_anim);
    
    current_index = next_index;
}
