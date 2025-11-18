#include "ResearchFormPage.h"
#include <polish_fonts.h>
#include <Arduino.h>
#include <PageID.h>
#include <PageNavigator.h>

// External page navigator reference (defined in main.cpp)
extern PageNavigator navigator;

// ==================== QUESTION DEFINITIONS (SINGLE SOURCE OF TRUTH) ====================

// Age options
static const char *age_options[] = {"poniżej 14 lat", "15-19", "20-25", "26-35", "36-45", "46-60", "powyżej 60 lat"};

// Gender options
static const char *gender_options[] = {"kobieta", "mężczyzna"};

// General education options
static const char *general_edu_options[] = {"podstawowe", "średnie ogólne", "średnie techniczne", "wyższe", "nie chcę podawać"};

// Musical education options
static const char *musical_edu_options[] = {
    "brak wykształcenia\nmuzycznego",
    "szkoła muzyczna\nI stopnia",
    "szkoła muzyczna\nII stopnia",
    "student akademii\nmuzycznej",
    "absolwent akademii\nmuzycznej",
    "samodzielne\nwykształcenie"
};

// Feeling adjectives (24 items)
static const char *feeling_adjectives[] = {
    "radośnie", "smutno", "zmęczony/a", "pogodnie", "znudzony/a", "dobrze",
    "rozbawiony/a", "nieswojo", "zestresowany/a", "zlękniony/a",
    "niewyspany/a", "źle", "rozgniewany/a", "pobudzony/a", "ponuro",
    "spięty/a", "usatysfakcjonowany/a", "zniecierpliwiony/a", "spokojnie",
    "stremowany/a", "przygnębiony/a", "komfortowo", "poirytowany/a", "zrelaksowany/a"
};

// Feeling scale options
static const char *feeling_scale[] = {"nie", "trochę", "tak", "bardzo"};

// Concentration options
static const char *concentration_options[] = {"bardzo wysoko", "wysoko", "raczej wysoko", "trudno powiedzieć", "raczej nisko", "nisko", "bardzo nisko"};

// Concert frequency options
static const char *concert_freq_options[] = {"kilka razy w tygodniu", "kilka razy w miesiącu", "kilka razy w roku", "około raz w roku", "rzadziej"};

// Music preferences options
static const char *music_pref_options[] = {"muzyka klasyczna", "muzyka filmowa", "muzyka popularna", "muzyka jazzowa", "muzyka kościelna"};

// ==================== STATIC MEMBER INITIALIZATION ====================

// Static member initialization
lv_obj_t* ResearchFormPage::screen = nullptr;
lv_obj_t* ResearchFormPage::scroll_container = nullptr;
lv_obj_t* ResearchFormPage::back_btn = nullptr;
lv_obj_t* ResearchFormPage::next_btn = nullptr;
int ResearchFormPage::current_page = 0;
const int ResearchFormPage::total_pages = 32;  // Intro + 4 simple questions + 24 feeling questions + 3 more questions (0-31)

// Dropdowns
lv_obj_t* ResearchFormPage::age_dropdown = nullptr;
lv_obj_t* ResearchFormPage::gender_dropdown = nullptr;
lv_obj_t* ResearchFormPage::general_education_dropdown = nullptr;
lv_obj_t* ResearchFormPage::musical_education_dropdown = nullptr;
lv_obj_t* ResearchFormPage::concentration_dropdown = nullptr;
lv_obj_t* ResearchFormPage::concert_frequency_dropdown = nullptr;

// Answer storage: -1 means not answered, 0+ is the selected option index
int ResearchFormPage::answers[32];  // Page 0 is intro (no answer), pages 1-31 have answers

// ==================== HELPER FUNCTIONS ====================

Question* ResearchFormPage::get_question(int page) {
    static Question questions[31];  // Pages 1-31
    static bool initialized = false;
    
    if (!initialized) {
        // Page 1: Age
        questions[1] = {"wiek", "Wiek", age_options, 7};
        
        // Page 2: Gender
        questions[2] = {"płeć", "Płeć", gender_options, 2};
        
        // Page 3: General education
        questions[3] = {"wykształcenie_ogólne", "Wykształcenie ogólne", general_edu_options, 5};
        
        // Page 4: Musical education
        questions[4] = {"wykształcenie_muzyczne", "Wykształcenie muzyczne", musical_edu_options, 6};
        
        // Pages 5-28: Feeling adjectives (24 questions)
        for (int i = 0; i < 24; i++) {
            questions[5 + i] = {feeling_adjectives[i], feeling_adjectives[i], feeling_scale, 5};
        }
        
        // Page 29: Concentration
        questions[29] = {"poziom_skupienia", "Poziom skupienia dzisiaj", concentration_options, 7};
        
        // Page 30: Concert frequency
        questions[30] = {"częstość_koncertów", "Jak często bywasz\nna koncertach?", concert_freq_options, 5};
        
        // Page 31: Music preferences
        questions[31] = {"muzyka_na_co_dzień", "Jakiej muzyki słuchasz\nna co dzień?", music_pref_options, 5};
        
        initialized = true;
    }
    
    if (page >= 1 && page <= 31) {
        return &questions[page];
    }
    return nullptr;
}

const char* ResearchFormPage::get_feeling_header(int page) {
    if (page >= 5 && page <= 28) {
        return "Jak się dzisiaj czujesz?";
    }
    return nullptr;
}

// ==================== PAGE CREATION ====================

lv_obj_t* ResearchFormPage::create() {
    // Initialize answers array to -1 (not answered)
    for (int i = 0; i < 32; i++) {
        answers[i] = -1;
    }
    
    // Create screen with white background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_WHITE), 0);
    
    // Create scrollable container for content (full height for questions)
    scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 310, 195);  // Full height minus buttons
    lv_obj_align(scroll_container, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(scroll_container, 5, 0);
    
    // Create navigation buttons at bottom
    back_btn = lv_button_create(screen);
    lv_obj_set_size(back_btn, 150, 40);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_add_event_cb(back_btn, on_back_clicked, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "<");
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_24, 0);  // Use standard font for symbols
    lv_obj_center(back_label);
    
    next_btn = lv_button_create(screen);
    lv_obj_set_size(next_btn, 150, 40);
    lv_obj_align(next_btn, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_obj_add_event_cb(next_btn, on_next_clicked, LV_EVENT_CLICKED, NULL);
    
    // Button gradient matching BeforeConcertPage (0xFF4B4B -> 0xFA6737)
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0xFF4B4B), 0);
    lv_obj_set_style_bg_grad_color(next_btn, lv_color_hex(0xFA6737), 0);
    lv_obj_set_style_bg_grad_dir(next_btn, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_width(next_btn, 0, 0);
    
    lv_obj_t *next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, ">");
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_24, 0);  // Use standard font for symbols
    lv_obj_set_style_text_color(next_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_center(next_label);
    
    // Show intro screen first
    create_intro_screen();
    
    // Disable back button on first page
    lv_obj_add_state(back_btn, LV_STATE_DISABLED);
    
    return screen;
}

void ResearchFormPage::create_intro_screen() {
    // Clear scroll container
    lv_obj_clean(scroll_container);
    
    // Add header "Formularz Badawczy" (larger)
    lv_obj_t *header = lv_label_create(scroll_container);
    lv_label_set_text(header, "Formularz Badawczy");
    lv_obj_set_style_text_color(header, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(header, &montserrat_24_polish, 0);  // Larger font
    lv_obj_set_style_text_align(header, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);
    
    // Add intro text
    lv_obj_t *intro_text = lv_label_create(scroll_container);
    lv_label_set_text(intro_text, 
        "Cześć! Dziękujemy za wysłuchanie koncertu i udział w badaniu :D\n\n"
        "Na sam koniec jeszcze tylko kilka krótkich pytań, które pozwolą nam "
        "lepiej zrozumieć - co czują wrocławianie?");
    lv_obj_set_style_text_color(intro_text, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(intro_text, &montserrat_14_polish, 0);
    lv_label_set_long_mode(intro_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(intro_text, 280);
    lv_obj_set_style_text_align(intro_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(intro_text, LV_ALIGN_TOP_MID, 0, 50);  // Below header
}

void ResearchFormPage::create_form_screen() {
    // Clear scroll container
    lv_obj_clean(scroll_container);
    
    // Get question data for current page
    Question *question = get_question(current_page);
    if (!question) return;
    
    const char *header_text = get_feeling_header(current_page);
    
    // For feeling questions (pages 5-28), add header above the adjective
    if (header_text) {
        lv_obj_t *header_label = lv_label_create(scroll_container);
        lv_label_set_text(header_label, header_text);
        lv_obj_set_style_text_color(header_label, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_set_style_text_font(header_label, &montserrat_14_polish, 0);  // Smaller font for header
        lv_obj_set_style_text_align(header_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_long_mode(header_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(header_label, 290);
        lv_obj_align(header_label, LV_ALIGN_TOP_MID, 0, 5);
    }
    
    // Create question label (larger, higher position)
    lv_obj_t *question_label = lv_label_create(scroll_container);
    lv_label_set_text(question_label, question->text);
    lv_obj_set_style_text_color(question_label, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(question_label, &montserrat_24_polish, 0);  // Larger font
    lv_obj_set_style_text_align(question_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(question_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(question_label, 290);
    lv_obj_align(question_label, LV_ALIGN_TOP_MID, 0, header_text ? 25 : 5);
    
    // Create radio button group
    int y_offset = 65;  // Extra space above first checkbox to avoid collision with 2-line questions
    for (int i = 0; i < question->option_count; i++) {
        // Create radio button (checkbox styled as radio)
        lv_obj_t *radio = lv_checkbox_create(scroll_container);
        lv_checkbox_set_text(radio, question->options[i]);
        lv_obj_set_style_text_font(radio, &montserrat_14_polish, 0);  // Larger font
        lv_obj_set_style_text_color(radio, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_align(radio, LV_ALIGN_TOP_LEFT, 10, y_offset);
        
        // Make checkbox indicator much larger
        lv_obj_set_style_width(radio, 32, LV_PART_INDICATOR);
        lv_obj_set_style_height(radio, 32, LV_PART_INDICATOR);
        
        // Check if this was previously selected
        if (answers[current_page] == i) {
            lv_obj_add_state(radio, LV_STATE_CHECKED);
        }
        
        // Make it behave like radio button (only one selected)
        lv_obj_add_event_cb(radio, on_dropdown_changed, LV_EVENT_VALUE_CHANGED, (void*)(intptr_t)i);
        
        y_offset += 32;  // Space between options
    }
    
    // Add extra space after last checkbox by creating invisible spacer
    lv_obj_t *spacer = lv_obj_create(scroll_container);
    lv_obj_set_size(spacer, 1, 40);  // 40px invisible spacer
    lv_obj_align(spacer, LV_ALIGN_TOP_LEFT, 0, y_offset);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spacer, 0, 0);
}

void ResearchFormPage::on_back_clicked(lv_event_t *e) {
    if (current_page > 0) {
        current_page--;
        
        if (current_page == 0) {
            create_intro_screen();
            lv_obj_add_state(back_btn, LV_STATE_DISABLED);
        } else {
            create_form_screen();
            lv_obj_clear_state(back_btn, LV_STATE_DISABLED);
        }
        
        // Update next button label
        lv_obj_t *next_label = lv_obj_get_child(next_btn, 0);
        if (current_page == total_pages - 1) {
            lv_label_set_text(next_label, "Wyślij");
            lv_obj_set_style_text_font(next_label, &montserrat_20_polish, 0);  // Polish font for "Wyślij"
        } else {
            lv_label_set_text(next_label, ">");
            lv_obj_set_style_text_font(next_label, &lv_font_montserrat_24, 0);  // Standard font for ">"
        }
        lv_obj_clear_state(next_btn, LV_STATE_DISABLED);
    }
}

void ResearchFormPage::on_next_clicked(lv_event_t *e) {
    if (current_page < total_pages - 1) {
        current_page++;
        
        if (current_page >= 1) {
            create_form_screen();
            lv_obj_clear_state(back_btn, LV_STATE_DISABLED);
        }
        
        // On last page, change button to "Wyślij"
        if (current_page == total_pages - 1) {
            lv_obj_t *next_label = lv_obj_get_child(next_btn, 0);
            lv_label_set_text(next_label, "Wyślij");
            lv_obj_set_style_text_font(next_label, &montserrat_20_polish, 0);  // Polish font for "Wyślij"
        }
    } else {
        // Submit form
        on_submit_clicked(e);
    }
}

void ResearchFormPage::on_submit_clicked(lv_event_t *e) {
    Serial.println("Form submitted!");
    Serial.println("Creating JSON from answers...");
    
    // Build JSON string
    String json = "{\n";
    bool first_item = true;
    
    // Regular questions (pages 1-4)
    for (int page = 1; page <= 4; page++) {
        if (answers[page] >= 0) {
            Question *q = get_question(page);
            if (q) {
                if (!first_item) json += ",\n";
                json += "  \"" + String(q->key) + "\": \"" + String(q->options[answers[page]]) + "\"";
                first_item = false;
            }
        }
    }
    
    // Feeling questions (pages 5-28) - nested object
    json += ",\n  \"jak_się_czujesz\": {\n";
    bool first_feeling = true;
    for (int page = 5; page <= 28; page++) {
        if (answers[page] >= 0) {
            Question *q = get_question(page);
            if (q) {
                if (!first_feeling) json += ",\n";
                json += "    \"" + String(q->key) + "\": \"" + String(q->options[answers[page]]) + "\"";
                first_feeling = false;
            }
        }
    }
    json += "\n  }";
    
    // Remaining questions (pages 29-31)
    for (int page = 29; page <= 31; page++) {
        if (answers[page] >= 0) {
            Question *q = get_question(page);
            if (q) {
                json += ",\n";
                json += "  \"" + String(q->key) + "\": \"" + String(q->options[answers[page]]) + "\"";
            }
        }
    }
    
    json += "\n}";
    
    // Print JSON to serial
    Serial.println("\n=== FORM DATA JSON ===");
    Serial.println(json);
    Serial.println("======================\n");
    
    // TODO: Send JSON via HTTP/MQTT
    
    // Navigate back to BEFORE_CONCERT page
    // Serial.println("Navigating back to BEFORE_CONCERT");
    // navigator.showPage(BEFORE_CONCERT);
}
void ResearchFormPage::on_dropdown_changed(lv_event_t *e) {
    lv_obj_t *checkbox = (lv_obj_t*)lv_event_get_target(e);
    int option_index = (int)(intptr_t)lv_event_get_user_data(e);
    
    // Get the parent container
    lv_obj_t *container = lv_obj_get_parent(checkbox);
    
    // If this checkbox was just checked, uncheck all others (radio button behavior)
    if (lv_obj_has_state(checkbox, LV_STATE_CHECKED)) {
        uint32_t child_count = lv_obj_get_child_count(container);
        for (uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(container, i);
            if (child != checkbox && lv_obj_check_type(child, &lv_checkbox_class)) {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
            }
        }
        
        // Store the answer for the current page
        answers[current_page] = option_index;
        
        Serial.print("Page ");
        Serial.print(current_page);
        Serial.print(" - Selected option: ");
        Serial.println(option_index);
    }
}

void ResearchFormPage::cleanup() {
    current_page = 0;
    
    // Reset answers
    for (int i = 0; i < 32; i++) {
        answers[i] = -1;
    }
    
    screen = nullptr;
    scroll_container = nullptr;
    back_btn = nullptr;
    next_btn = nullptr;
    age_dropdown = nullptr;
    gender_dropdown = nullptr;
    general_education_dropdown = nullptr;
    musical_education_dropdown = nullptr;
    concentration_dropdown = nullptr;
    concert_frequency_dropdown = nullptr;
}
