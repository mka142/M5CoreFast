#include "SliderDemoPage.h"
#include <polish_fonts.h>
#include <stdio.h>

lv_obj_t* SliderDemoPage::value_label = nullptr;

lv_obj_t* SliderDemoPage::create() {
    // Create screen
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), 0);
    
    // Title
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Test Suwaka");
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(title, &montserrat_24_polish, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create slider
    lv_obj_t *slider = lv_slider_create(screen);
    lv_obj_set_width(slider, 250);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    
    // Style the slider
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_SURFACE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_PRIMARY), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(COLOR_ACCENT), LV_PART_KNOB);
    
    // Value label
    value_label = lv_label_create(screen);
    lv_label_set_text(value_label, "50");
    lv_obj_set_style_text_color(value_label, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(value_label, &montserrat_32_polish, 0);
    lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -60);
    
    // Add event callback
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    return screen;
}

void SliderDemoPage::slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider = (lv_obj_t*)lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    
    if (value_label) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", value);
        lv_label_set_text(value_label, buf);
    }
}
