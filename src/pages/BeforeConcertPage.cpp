#include "BeforeConcertPage.h"
#include <polish_fonts.h>
#include <cmath>

lv_obj_t* BeforeConcertPage::screen = nullptr;
lv_obj_t* BeforeConcertPage::title_label = nullptr;
int BeforeConcertPage::pulse_value = 0;

lv_obj_t* BeforeConcertPage::create() {
    // Create screen with dark background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), 0);
    
    // Main question
    title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "Co czują Wrocławianie?");
    lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(title_label, &montserrat_24_polish, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, 280);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    
    // Subtitle
    lv_obj_t *subtitle = lv_label_create(screen);
    lv_label_set_text(subtitle, "Przygotowanie do koncertu...");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(COLOR_GRAY), 0);
    lv_obj_set_style_text_font(subtitle, &montserrat_14_polish, 0);
    lv_obj_align(subtitle, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    // Create timer for pulsing effect
    lv_timer_create(update_pulse, 50, NULL);
    
    return screen;
}

void BeforeConcertPage::update_pulse(lv_timer_t *timer) {
    if (title_label) {
        pulse_value = (pulse_value + 5) % 360;
        // Simple color pulse effect - vary blue component
        int blue = 128 + (int)(127 * sin(pulse_value * 3.14159 / 180.0));
        lv_obj_set_style_text_color(title_label, lv_color_make(30, 144, blue), 0);
    }
}
