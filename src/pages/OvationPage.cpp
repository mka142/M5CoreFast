#include "OvationPage.h"
#include <polish_fonts.h>

// Static member definitions
lv_obj_t* OvationPage::message_label = nullptr;
lv_obj_t* OvationPage::dot1 = nullptr;
lv_obj_t* OvationPage::dot2 = nullptr;
lv_obj_t* OvationPage::dot3 = nullptr;
int OvationPage::animation_step = 0;
lv_timer_t* OvationPage::animation_timer = nullptr;
EventSchema OvationPage::eventPayload = {};  // Initialize payload storage

lv_obj_t* OvationPage::create() {
    // Create main screen
    lv_obj_t* screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // Message label (centered, white)
    message_label = lv_label_create(screen);
    lv_obj_set_style_text_font(message_label, &montserrat_24_polish, 0);
    lv_obj_set_style_text_color(message_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(message_label, 280);
    lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(message_label, "Owacja");
    lv_obj_align(message_label, LV_ALIGN_CENTER, 0, -20);
    
    // Create three dots below the message
    // Dot 1
    dot1 = lv_label_create(screen);
    lv_obj_set_style_text_font(dot1, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(dot1, lv_color_hex(0x999999), 0);
    lv_label_set_text(dot1, "•");
    lv_obj_align(dot1, LV_ALIGN_CENTER, -50, 60);
    lv_obj_set_style_opa(dot1, LV_OPA_30, 0);
    
    // Dot 2
    dot2 = lv_label_create(screen);
    lv_obj_set_style_text_font(dot2, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(dot2, lv_color_hex(0x999999), 0);
    lv_label_set_text(dot2, "•");
    lv_obj_align(dot2, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_opa(dot2, LV_OPA_30, 0);
    
    // Dot 3
    dot3 = lv_label_create(screen);
    lv_obj_set_style_text_font(dot3, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(dot3, lv_color_hex(0x999999), 0);
    lv_label_set_text(dot3, "•");
    lv_obj_align(dot3, LV_ALIGN_CENTER, 50, 60);
    lv_obj_set_style_opa(dot3, LV_OPA_30, 0);
    
    // Create timer for animation (600ms interval - slower pulsing)
    animation_step = 0;
    animation_timer = lv_timer_create(update_animation, 600, nullptr);
    
    return screen;
}

void OvationPage::update_animation(lv_timer_t *timer) {
    animation_step = (animation_step + 1) % 4;
    
    // Pulsing pattern: each dot fades in and out in sequence
    switch (animation_step) {
        case 0:
            lv_obj_set_style_opa(dot1, LV_OPA_100, 0);
            lv_obj_set_style_opa(dot2, LV_OPA_30, 0);
            lv_obj_set_style_opa(dot3, LV_OPA_30, 0);
            break;
        case 1:
            lv_obj_set_style_opa(dot1, LV_OPA_100, 0);
            lv_obj_set_style_opa(dot2, LV_OPA_100, 0);
            lv_obj_set_style_opa(dot3, LV_OPA_30, 0);
            break;
        case 2:
            lv_obj_set_style_opa(dot1, LV_OPA_100, 0);
            lv_obj_set_style_opa(dot2, LV_OPA_100, 0);
            lv_obj_set_style_opa(dot3, LV_OPA_100, 0);
            break;
        case 3:
            lv_obj_set_style_opa(dot1, LV_OPA_30, 0);
            lv_obj_set_style_opa(dot2, LV_OPA_30, 0);
            lv_obj_set_style_opa(dot3, LV_OPA_30, 0);
            break;
    }
}

void OvationPage::setMessage(const char* message) {
    if (message_label) {
        lv_label_set_text(message_label, message);
    }
}

void OvationPage::setPayload(const EventSchema& payload) {
    eventPayload = payload;
    
    // Update UI with payload data
    if (!payload.payload.isNull() && payload.payload["message"].is<const char*>()) {
        const char* message = payload.payload["message"];
        setMessage(message);
    } else if (!payload.label.empty()) {
        setMessage(payload.label.c_str());
    } else {
        setMessage("Owacja");
    }
    
    Serial.println("=== OvationPage setPayload ===");
    Serial.print("Concert ID: ");
    Serial.println(payload.concertId.c_str());
    Serial.print("Event Type: ");
    Serial.println(payload.eventType.c_str());
    Serial.print("Label: ");
    Serial.println(payload.label.c_str());
}

const EventSchema& OvationPage::getPayload() {
    return eventPayload;
}
