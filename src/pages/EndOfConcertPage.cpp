#include "EndOfConcertPage.h"
#include <polish_fonts.h>
#include <PageID.h>
#include <PageNavigator.h>
#include <HTTPAdapter.h>

// External references (defined in main.cpp)
extern PageNavigator navigator;
extern HTTPAdapter httpAdapter;
extern const char *USER_ID_STR;
extern const char *EXAM_FORM_GET_RESPONSE_ENDPOINT;
extern const char *FORM_ID_FEEDBACK;

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

void EndOfConcertPage::firstRender() {
    // Check form submission status when page is shown
    updateFormButtonState();
}

void EndOfConcertPage::lastRender() {
    // Nothing to do on last render for this page
}

bool EndOfConcertPage::checkFormSubmitted() {
    // Build URL with userId and formId query parameters
    String url = String(EXAM_FORM_GET_RESPONSE_ENDPOINT);
    url += "/";
    url += USER_ID_STR;
    url += "/form/";
    url += FORM_ID_FEEDBACK;
    
    Serial.print("Checking feedback form submission status: ");
    Serial.println(url);
    
    int httpCode = httpAdapter.get(url.c_str());
    
    if (httpCode == 200) {
        Serial.println("Feedback form already submitted - navigating to form finished page");
        // If form is already submitted, navigate directly to FormFinishedPage
        navigator.showPage(END_OF_CONCERT__FORM_FINISHED);
        return true;
    } else {
        Serial.printf("Feedback form not submitted (HTTP code: %d)\n", httpCode);
        return false;
    }
}

void EndOfConcertPage::updateFormButtonState() {
    if (!button) return;
    
    bool formSubmitted = checkFormSubmitted();
    
    if (formSubmitted) {
        // Form is submitted - navigation to FormFinishedPage already happened in checkFormSubmitted()
        // Disable button anyway in case we return to this page
        lv_obj_add_state(button, LV_STATE_DISABLED);
        
        // Change button text
        lv_obj_t *button_label = lv_obj_get_child(button, 0);
        if (button_label) {
            lv_label_set_text(button_label, "Formularz wypełniony");
        }
        
        // Change button color to gray
        lv_obj_set_style_bg_color(button, lv_color_hex(0x808080), 0);
        lv_obj_set_style_bg_grad_color(button, lv_color_hex(0x606060), 0);
        
        Serial.println("Feedback form button disabled - already submitted");
    } else {
        // Enable button (if it was disabled)
        lv_obj_clear_state(button, LV_STATE_DISABLED);
        Serial.println("Feedback form button enabled - not yet submitted");
    }
}
