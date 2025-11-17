#include "LoadingPage.h"
#include <polish_fonts.h>

lv_obj_t* LoadingPage::label_dots = nullptr;
int LoadingPage::dot_count = 0;

lv_obj_t* LoadingPage::create() {
    // Create screen
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BLACK), 0);
    
    // Main title
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "Ładowanie");
    lv_obj_set_style_text_color(label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(label, &montserrat_24_polish, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);
    
    // Animated dots
    label_dots = lv_label_create(screen);
    lv_label_set_text(label_dots, "");
    lv_obj_set_style_text_color(label_dots, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(label_dots, &montserrat_24_polish, 0);
    lv_obj_align(label_dots, LV_ALIGN_CENTER, 0, 10);
    
    // Subtitle
    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "Proszę czekać...");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(COLOR_GRAY), 0);
    lv_obj_set_style_text_font(subtitle, &montserrat_14_polish, 0);
    lv_obj_align(subtitle, LV_ALIGN_CENTER, 0, 50);
    
    // Create timer for dot animation (1 second interval)
    lv_timer_create(update_animation, 1000, NULL);
    
    return screen;
}

void LoadingPage::update_animation(lv_timer_t *timer) {
    if (label_dots) {
        dot_count = (dot_count + 1) % 4;
        switch(dot_count) {
            case 0: lv_label_set_text(label_dots, ""); break;
            case 1: lv_label_set_text(label_dots, "."); break;
            case 2: lv_label_set_text(label_dots, ". ."); break;
            case 3: lv_label_set_text(label_dots, ". . ."); break;
        }
    }
}
