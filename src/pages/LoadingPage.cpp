#include "LoadingPage.h"
#include <polish_fonts.h>
#include <RGBAdapter.h>

// Declare external image
LV_IMG_DECLARE(note);

// Access the global RGB adapter instance declared in main.cpp
extern RGBAdapter rgb;

lv_obj_t* LoadingPage::note_img1 = nullptr;
lv_obj_t* LoadingPage::note_img2 = nullptr;
lv_obj_t* LoadingPage::note_img3 = nullptr;
int LoadingPage::animation_step = 0;

lv_obj_t* LoadingPage::create() {
    // Create screen with black background
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BLACK), 0);
    
    // Create three note images (initially hidden)
    // Note 1 (left)
    note_img1 = lv_image_create(screen);
    lv_image_set_src(note_img1, &note);
    lv_obj_set_style_image_recolor(note_img1, lv_color_hex(0x42b2c2), 0);  // Cyan color
    lv_obj_set_style_image_recolor_opa(note_img1, LV_OPA_COVER, 0);
    lv_image_set_scale(note_img1, 180);  // Scale down to 70% (256 = 100%)
    lv_obj_align(note_img1, LV_ALIGN_CENTER, -60, -20);
    lv_obj_add_flag(note_img1, LV_OBJ_FLAG_HIDDEN);
    
    // Note 2 (center)
    note_img2 = lv_image_create(screen);
    lv_image_set_src(note_img2, &note);
    lv_obj_set_style_image_recolor(note_img2, lv_color_hex(0x42b2c2), 0);  // Cyan color
    lv_obj_set_style_image_recolor_opa(note_img2, LV_OPA_COVER, 0);
    lv_image_set_scale(note_img2, 180);  // Scale down to 70%
    lv_obj_align(note_img2, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_flag(note_img2, LV_OBJ_FLAG_HIDDEN);
    
    // Note 3 (right)
    note_img3 = lv_image_create(screen);
    lv_image_set_src(note_img3, &note);
    lv_obj_set_style_image_recolor(note_img3, lv_color_hex(0x42b2c2), 0);  // Cyan color
    lv_obj_set_style_image_recolor_opa(note_img3, LV_OPA_COVER, 0);
    lv_image_set_scale(note_img3, 180);  // Scale down to 70%
    lv_obj_align(note_img3, LV_ALIGN_CENTER, 60, -20);
    lv_obj_add_flag(note_img3, LV_OBJ_FLAG_HIDDEN);
    
    // "Ładowanie..." text below notes
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Ładowanie...");
    lv_obj_set_style_text_color(label, lv_color_hex(0x42b2c2), 0);
    lv_obj_set_style_text_font(label, &montserrat_24_polish, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);
    
    // Create timer for note animation
    // Step 0: 500ms - show 1 note
    // Step 1: 1000ms - show 2 notes
    // Step 2: 1000ms - show 3 notes
    // Step 3: 1000ms - hide all
    lv_timer_create(update_animation, 500, NULL);
    
    return screen;
}

void LoadingPage::firstRender() {
    // Turn off RGB LEDs for loading page
    rgb.setColor(0, 0, 0);
}

void LoadingPage::lastRender() {
    // No special cleanup needed
}


void LoadingPage::update_animation(lv_timer_t *timer) {
    if (!note_img1 || !note_img2 || !note_img3) return;
    
    switch(animation_step) {
        case 0:  // Fade in note 1 only
            lv_obj_clear_flag(note_img1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(note_img1, LV_OPA_COVER, 0);
            lv_obj_add_flag(note_img2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(note_img3, LV_OBJ_FLAG_HIDDEN);
            lv_timer_set_period(timer, 1000);
            break;
            
        case 1:  // Fade in note 2 (note 1 stays visible)
            lv_obj_clear_flag(note_img2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(note_img2, LV_OPA_COVER, 0);
            break;
            
        case 2:  // Fade in note 3 (notes 1 & 2 stay visible)
            lv_obj_clear_flag(note_img3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_opa(note_img3, LV_OPA_COVER, 0);
            break;
            
        case 3:  // Fade out all notes
            lv_obj_set_style_opa(note_img1, LV_OPA_TRANSP, 0);
            lv_obj_set_style_opa(note_img2, LV_OPA_TRANSP, 0);
            lv_obj_set_style_opa(note_img3, LV_OPA_TRANSP, 0);
            lv_timer_set_period(timer, 500);
            break;
    }
    
    animation_step = (animation_step + 1) % 4;
}
