#pragma once
#include <Arduino.h>
#include <lvgl.h>

// Question structure
struct Question {
    const char* key;
    const char* text;
    const char** options;
    int option_count;
};

class FormPageBase {
protected:
    // UI elements
    static lv_obj_t* screen;
    static lv_obj_t* scroll_container;
    static lv_obj_t* back_btn;
    static lv_obj_t* next_btn;
    
    // Form state
    static int current_page;
    static int total_pages;
    static int* answers;  // Dynamic array
    
    // Abstract methods to be implemented by derived classes
    virtual void create_intro_screen() = 0;
    virtual Question* get_question(int page) = 0;
    virtual const char* get_section_header(int page) = 0;
    virtual void on_form_submit() = 0;
    virtual const char* get_intro_title() = 0;
    virtual const char* get_intro_text() = 0;
    
    // Common form methods
    void create_form_screen();
    void update_button_states();
    
    // Event handlers
    static void on_back_clicked(lv_event_t *e);
    static void on_next_clicked(lv_event_t *e);
    static void on_dropdown_changed(lv_event_t *e);
    
public:
    FormPageBase();
    virtual ~FormPageBase();
    
    // Common creation method
    lv_obj_t* create_base(int total_page_count);
    void cleanup_base();
};
