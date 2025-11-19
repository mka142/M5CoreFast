#include "EndOfConcertPage.h"
#include <polish_fonts.h>
#include <PageID.h>
#include <PageNavigator.h>

// External page navigator reference (defined in main.cpp)
extern PageNavigator navigator;

// Static member initialization
lv_obj_t* EndOfConcertPage::screen = nullptr;
lv_obj_t* EndOfConcertPage::main_label = nullptr;
lv_obj_t* EndOfConcertPage::description_label = nullptr;
lv_obj_t* EndOfConcertPage::button = nullptr;

lv_obj_t* EndOfConcertPage::create() {
    // Create screen with black background
    screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // Create scrollable container for content
    lv_obj_t* scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 320, 180);  // Leave 60px at bottom for button
    lv_obj_set_pos(scroll_container, 0, 0);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_style_pad_all(scroll_container, 20, 0);
    lv_obj_set_flex_flow(scroll_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(scroll_container, LV_DIR_VER);
    
    // Main text label (white, 32pt)
    main_label = lv_label_create(scroll_container);
    lv_label_set_text(main_label, "Podziel się swoimi wrażeniami");
    lv_obj_set_style_text_color(main_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(main_label, &montserrat_32_polish, 0);
    lv_obj_set_style_text_align(main_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(main_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(main_label, 280);
    
    // Description label (gray, 14pt)
    description_label = lv_label_create(scroll_container);
    lv_label_set_text(description_label, "Twoja opinia jest dla nas bardzo ważna. Wypełnienie krótkiej ankiety pomoże nam lepiej zrozumieć Twoje doświadczenia z tego koncertu badawczego.");
    lv_obj_set_style_text_color(description_label, lv_color_hex(0x999999), 0);
    lv_obj_set_style_text_font(description_label, &montserrat_14_polish, 0);
    lv_obj_set_style_text_align(description_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(description_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(description_label, 280);
    lv_obj_set_style_pad_top(description_label, 20, 0);
    lv_obj_set_style_pad_bottom(description_label, 20, 0);
    
    // Create gradient button at bottom (identical to BeforeConcertPage)
    button = lv_button_create(screen);
    lv_obj_set_size(button, 300, 50);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_radius(button, 25, 0);  // Rounded corners
    
    // Button gradient (0xFF4B4B -> 0xFA6737)
    lv_obj_set_style_bg_color(button, lv_color_hex(0xFF4B4B), 0);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0xFA6737), 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    
    // Button label
    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, "Otwórz formularz");
    lv_obj_set_style_text_color(button_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(button_label, &montserrat_20_polish, 0);
    lv_obj_center(button_label);
    
    // Add button click event handler
    lv_obj_add_event_cb(button, on_form_button_clicked, LV_EVENT_CLICKED, nullptr);
    
    return screen;
}

void EndOfConcertPage::on_form_button_clicked(lv_event_t *e) {
    Serial.println("End of concert form button clicked - navigating to END_OF_CONCERT__FEEDBACK_FORM");
    navigator.showPage(END_OF_CONCERT__FEEDBACK_FORM);
}
