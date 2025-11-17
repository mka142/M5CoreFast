#include "TensionMeasurementPage.h"
#include <polish_fonts.h>
#include <stdio.h>

lv_obj_t* TensionMeasurementPage::value_label = nullptr;
lv_obj_t* TensionMeasurementPage::buffer_label = nullptr;

lv_obj_t* TensionMeasurementPage::create() {
    // Create screen with yellow tint for measurement mode
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BACKGROUND), 0);
    
    // Title
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Pomiar Napięcia");
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_YELLOW), 0);
    lv_obj_set_style_text_font(title, &montserrat_20_polish, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    // Large value display
    value_label = lv_label_create(screen);
    lv_label_set_text(value_label, "0");
    lv_obj_set_style_text_color(value_label, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(value_label, &montserrat_48_polish, 0);
    lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -20);
    
    // Buffer status
    buffer_label = lv_label_create(screen);
    lv_label_set_text(buffer_label, "Buffer: 0/200");
    lv_obj_set_style_text_color(buffer_label, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_style_text_font(buffer_label, &montserrat_14_polish, 0);
    lv_obj_align(buffer_label, LV_ALIGN_BOTTOM_MID, 0, -40);
    
    // Instructions
    lv_obj_t *instructions = lv_label_create(screen);
    lv_label_set_text(instructions, "Obróć pokrętłem");
    lv_obj_set_style_text_color(instructions, lv_color_hex(COLOR_GRAY), 0);
    lv_obj_set_style_text_font(instructions, &montserrat_12_polish, 0);
    lv_obj_align(instructions, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    return screen;
}

void TensionMeasurementPage::updateValue(int value) {
    if (value_label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", value);
        lv_label_set_text(value_label, buf);
    }
}

void TensionMeasurementPage::updateBuffer(int count, int max) {
    if (buffer_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Buffer: %d/%d", count, max);
        lv_label_set_text(buffer_label, buf);
    }
}
