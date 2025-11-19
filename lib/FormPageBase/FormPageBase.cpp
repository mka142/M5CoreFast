#include "FormPageBase.h"
#include <polish_fonts.h>
#include <ThemeColors.h>

// Static member initialization
lv_obj_t* FormPageBase::screen = nullptr;
lv_obj_t* FormPageBase::scroll_container = nullptr;
lv_obj_t* FormPageBase::back_btn = nullptr;
lv_obj_t* FormPageBase::next_btn = nullptr;
int FormPageBase::current_page = 0;
int FormPageBase::total_pages = 0;
int* FormPageBase::answers = nullptr;

FormPageBase::FormPageBase() {}

FormPageBase::~FormPageBase() {
    if (answers) {
        delete[] answers;
        answers = nullptr;
    }
}

lv_obj_t* FormPageBase::create_base(int total_page_count) {
    total_pages = total_page_count;
    current_page = 0;
    
    // Initialize answers array
    if (answers) delete[] answers;
    answers = new int[total_pages];
    for (int i = 0; i < total_pages; i++) {
        answers[i] = -1;
    }
    
    // Create screen with white background
    screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_WHITE), 0);
    
    // Create scrollable container for content
    scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 310, 195);
    lv_obj_align(scroll_container, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(scroll_container, 5, 0);
    
    // Create navigation buttons at bottom
    back_btn = lv_button_create(screen);
    lv_obj_set_size(back_btn, 150, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_add_event_cb(back_btn, on_back_clicked, LV_EVENT_CLICKED, this);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "<");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);
    lv_obj_center(back_label);
    
    next_btn = lv_button_create(screen);
    lv_obj_set_size(next_btn, 150, 40);
    lv_obj_align(next_btn, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_obj_add_event_cb(next_btn, on_next_clicked, LV_EVENT_CLICKED, this);
    
    // Button gradient (0xFF4B4B -> 0xFA6737)
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0xFF4B4B), 0);
    lv_obj_set_style_bg_grad_color(next_btn, lv_color_hex(0xFA6737), 0);
    lv_obj_set_style_bg_grad_dir(next_btn, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_width(next_btn, 0, 0);
    
    lv_obj_t *next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, ">");
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(next_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_center(next_label);
    
    // Show intro screen first
    create_intro_screen();
    
    // Disable back button on first page
    lv_obj_add_state(back_btn, LV_STATE_DISABLED);
    
    return screen;
}

void FormPageBase::create_form_screen() {
    // Clear scroll container
    lv_obj_clean(scroll_container);
    
    // Get question data for current page
    Question *question = get_question(current_page);
    if (!question) return;
    
    const char *header_text = get_section_header(current_page);
    
    // Create a flex container for vertical layout
    lv_obj_t *content_container = lv_obj_create(scroll_container);
    lv_obj_set_size(content_container, 290, LV_SIZE_CONTENT);
    lv_obj_set_pos(content_container, 0, 0);
    lv_obj_set_style_bg_opa(content_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content_container, 0, 0);
    lv_obj_set_style_pad_all(content_container, 0, 0);
    lv_obj_set_flex_flow(content_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add section header if exists (with extra bottom padding)
    if (header_text) {
        lv_obj_t *header_label = lv_label_create(content_container);
        lv_label_set_text(header_label, header_text);
        lv_obj_set_style_text_color(header_label, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_set_style_text_font(header_label, &montserrat_14_polish, 0);
        lv_obj_set_style_text_align(header_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(header_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(header_label, 280);
        lv_obj_set_style_pad_bottom(header_label, 15, 0);  // Extra space below header
    }
    
    // Create question label (with extra bottom padding)
    lv_obj_t *question_label = lv_label_create(content_container);
    lv_label_set_text(question_label, question->text);
    lv_obj_set_style_text_color(question_label, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(question_label, &montserrat_24_polish, 0);
    lv_obj_set_style_text_align(question_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(question_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(question_label, 280);
    lv_obj_set_style_pad_bottom(question_label, 20, 0);  // Extra space below question
    
    // Create radio buttons container with flex layout
    lv_obj_t *radio_container = lv_obj_create(content_container);
    lv_obj_set_size(radio_container, 280, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(radio_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(radio_container, 0, 0);
    lv_obj_set_style_pad_all(radio_container, 0, 0);
    lv_obj_set_flex_flow(radio_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(radio_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    
    // Create radio buttons
    for (int i = 0; i < question->option_count; i++) {
        lv_obj_t *radio = lv_checkbox_create(radio_container);
        lv_checkbox_set_text(radio, question->options[i]);
        lv_obj_set_style_text_font(radio, &montserrat_14_polish, 0);
        lv_obj_set_style_text_color(radio, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_set_style_pad_bottom(radio, 8, 0);  // Space between radio buttons
        
        // Make checkbox indicator larger
        lv_obj_set_style_width(radio, 32, LV_PART_INDICATOR);
        lv_obj_set_style_height(radio, 32, LV_PART_INDICATOR);
        
        // Check if this was previously selected
        if (answers[current_page] == i) {
            lv_obj_add_state(radio, LV_STATE_CHECKED);
        }
        
        // Radio button behavior
        lv_obj_add_event_cb(radio, on_dropdown_changed, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);
    }
    
    // Add bottom spacer for scroll comfort
    lv_obj_t *spacer = lv_obj_create(content_container);
    lv_obj_set_size(spacer, 1, 30);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
}

void FormPageBase::update_button_states() {
    // Update back button
    if (current_page == 0) {
        lv_obj_add_state(back_btn, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(back_btn, LV_STATE_DISABLED);
    }
    
    // Update next button label
    lv_obj_t *next_label = lv_obj_get_child(next_btn, 0);
    if (current_page == total_pages - 1) {
        lv_label_set_text(next_label, "Wyślij");
        lv_obj_set_style_text_font(next_label, &montserrat_20_polish, 0);
    } else {
        lv_label_set_text(next_label, ">");
        lv_obj_set_style_text_font(next_label, &lv_font_montserrat_24, 0);
    }
}

void FormPageBase::on_back_clicked(lv_event_t *e) {
    FormPageBase* form = (FormPageBase*)lv_event_get_user_data(e);
    
    if (current_page > 0) {
        current_page--;
        
        if (current_page == 0) {
            form->create_intro_screen();
        } else {
            form->create_form_screen();
        }
        
        form->update_button_states();
    }
}

void FormPageBase::on_next_clicked(lv_event_t *e) {
    FormPageBase* form = (FormPageBase*)lv_event_get_user_data(e);
    
    if (current_page < total_pages - 1) {
        current_page++;
        
        if (current_page >= 1) {
            form->create_form_screen();
        }
        
        form->update_button_states();
    } else {
        // Submit form
        form->on_form_submit();
    }
}

void FormPageBase::on_dropdown_changed(lv_event_t *e) {
    lv_obj_t *checkbox = (lv_obj_t*)lv_event_get_target(e);
    int option_index = (int)(intptr_t)lv_event_get_user_data(e);
    
    lv_obj_t *container = lv_obj_get_parent(checkbox);
    
    // Uncheck all others (radio button behavior)
    if (lv_obj_has_state(checkbox, LV_STATE_CHECKED)) {
        uint32_t child_count = lv_obj_get_child_count(container);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(container, i);
            if (child != checkbox && lv_obj_check_type(child, &lv_checkbox_class)) {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
            }
        }
        
        answers[current_page] = option_index;
        
        Serial.print("Page ");
        Serial.print(current_page);
        Serial.print(" - Selected option: ");
        Serial.println(option_index);
    }
}

void FormPageBase::cleanup_base() {
    current_page = 0;
    
    if (answers) {
        delete[] answers;
        answers = nullptr;
    }
    
    screen = nullptr;
    scroll_container = nullptr;
    back_btn = nullptr;
    next_btn = nullptr;
}
