#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Question data structure
struct Question {
    const char *key;          // JSON key
    const char *text;         // Display text
    const char **options;     // Array of option texts
    int option_count;         // Number of options
};

class ResearchFormPage {
public:
    static lv_obj_t* create();
    static void cleanup();
    
    // Navigation callbacks
    static void on_back_clicked(lv_event_t *e);
    static void on_next_clicked(lv_event_t *e);
    static void on_submit_clicked(lv_event_t *e);
    
    // Dropdown callbacks
    static void on_dropdown_changed(lv_event_t *e);
    
private:
    static lv_obj_t *screen;
    static lv_obj_t *scroll_container;
    static lv_obj_t *back_btn;
    static lv_obj_t *next_btn;
    
    // Form state tracking
    static int current_page;
    static const int total_pages;
    
    // Dropdowns for answers
    static lv_obj_t *age_dropdown;
    static lv_obj_t *gender_dropdown;
    static lv_obj_t *general_education_dropdown;
    static lv_obj_t *musical_education_dropdown;
    static lv_obj_t *concentration_dropdown;
    static lv_obj_t *concert_frequency_dropdown;
    
    // Answer storage: -1 means not answered, 0+ is the selected option index
    static int answers[32];  // Page 0 is intro (no answer), pages 1-31 have answers
    
    // Question definitions
    static Question* get_question(int page);
    static const char* get_feeling_header(int page);
    
    // Helper functions
    static void create_intro_screen();
    static void create_form_screen();
};
