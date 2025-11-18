#include "ChargingPage.h"
#include <M5CoreS3.h>
#include <Arduino.h>

// Static member initialization
lv_obj_t* ChargingPage::screen = nullptr;
lv_obj_t* ChargingPage::battery_label = nullptr;
lv_obj_t* ChargingPage::percentage_label = nullptr;

const char* ChargingPage::get_battery_symbol(int percentage) {
    if (percentage > 75) {
        return LV_SYMBOL_BATTERY_FULL;
    } else if (percentage > 50) {
        return LV_SYMBOL_BATTERY_3;
    } else if (percentage > 25) {
        return LV_SYMBOL_BATTERY_2;
    } else if (percentage > 10) {
        return LV_SYMBOL_BATTERY_1;
    } else {
        return LV_SYMBOL_BATTERY_EMPTY;
    }
}

lv_obj_t* ChargingPage::create() {
    // Create screen with dark background (low brightness feel)
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x1a1a1a), 0);  // Very dark gray
    
    // Add click event to dismiss the screen
    lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(screen, on_screen_clicked, LV_EVENT_CLICKED, NULL);
    
    // Create battery symbol label (large, centered high)
    battery_label = lv_label_create(screen);
    lv_obj_set_style_text_font(battery_label, &lv_font_montserrat_48, 0);  // Large system font
    lv_obj_set_style_text_color(battery_label, lv_color_hex(0x808080), 0);  // Medium gray (not bright)
    lv_obj_align(battery_label, LV_ALIGN_CENTER, 0, -30);
    
    // Create percentage label (below battery symbol)
    percentage_label = lv_label_create(screen);
    lv_obj_set_style_text_font(percentage_label, &lv_font_montserrat_32, 0);  // Medium system font
    lv_obj_set_style_text_color(percentage_label, lv_color_hex(0x808080), 0);  // Medium gray
    lv_obj_align(percentage_label, LV_ALIGN_CENTER, 0, 40);
    
    // Initial battery status
    update_battery_status();
    
    return screen;
}

void ChargingPage::update_battery_status() {
    if (!battery_label || !percentage_label) return;
    
    // Get battery level from M5CoreS3
    int battery_level = M5.Power.getBatteryLevel();
    
    // Update battery symbol
    const char* symbol = get_battery_symbol(battery_level);
    lv_label_set_text(battery_label, symbol);
    
    // Update percentage text
    char percent_text[8];
    snprintf(percent_text, sizeof(percent_text), "%d%%", battery_level);
    lv_label_set_text(percentage_label, percent_text);
    
    // Re-center both labels
    lv_obj_align(battery_label, LV_ALIGN_CENTER, 0, -30);
    lv_obj_align(percentage_label, LV_ALIGN_CENTER, 0, 40);
}

void ChargingPage::on_screen_clicked(lv_event_t *e) {
    Serial.println("Charging screen dismissed by user click");
    // The main loop will handle returning to normal operation
}

void ChargingPage::cleanup() {
    screen = nullptr;
    battery_label = nullptr;
    percentage_label = nullptr;
}
