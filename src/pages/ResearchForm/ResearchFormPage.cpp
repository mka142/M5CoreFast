#include "ResearchFormPage.h"
#include <polish_fonts.h>
#include <Arduino.h>
#include <PageID.h>
#include <PageNavigator.h>
#include <ThemeColors.h>

// External page navigator reference (defined in main.cpp)
extern PageNavigator navigator;

// Static instance
ResearchFormPage* ResearchFormPage::instance = nullptr;

// ==================== QUESTION DEFINITIONS ====================

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

// ==================== PUBLIC INTERFACE ====================

lv_obj_t* ResearchFormPage::create() {
    if (!instance) {
        instance = new ResearchFormPage();
    }
    return instance->create_base(32);  // 32 total pages (0=intro, 1-31=questions)
}

void ResearchFormPage::cleanup() {
    if (instance) {
        instance->cleanup_base();
        delete instance;
        instance = nullptr;
    }
}

// ==================== PROTECTED IMPLEMENTATIONS ====================

const char* ResearchFormPage::get_intro_title() {
    return "Formularz Badawczy";
}

const char* ResearchFormPage::get_intro_text() {
    return "Witamy na koncercie!\n\n"
           "Dziękujemy za udział w badaniu \"Co czują Wrocławianie?\".\n\n"
           "Przed koncertem prosimy o wypełnienie krótkiego formularza, "
           "który pomoże nam lepiej zrozumieć Państwa odczucia.";
}

void ResearchFormPage::create_intro_screen() {
    // Clear scroll container
    lv_obj_clean(scroll_container);
    
    // Add header
    lv_obj_t *header = lv_label_create(scroll_container);
    lv_label_set_text(header, get_intro_title());
    lv_obj_set_style_text_color(header, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(header, &montserrat_24_polish, 0);
    lv_obj_set_style_text_align(header, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 10);
    
    // Add intro text
    lv_obj_t *intro_text = lv_label_create(scroll_container);
    lv_label_set_text(intro_text, get_intro_text());
    lv_obj_set_style_text_color(intro_text, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_text_font(intro_text, &montserrat_14_polish, 0);
    lv_label_set_long_mode(intro_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(intro_text, 280);
    lv_obj_set_style_text_align(intro_text, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(intro_text, LV_ALIGN_TOP_MID, 0, 50);
}

Question* ResearchFormPage::get_question(int page) {
    Serial.printf("[ResearchForm] get_question called with page=%d\n", page);
    
    static Question questions[32];  // Needs 32 elements for indices 0-31 (page 0 unused, pages 1-31 used)
    static bool initialized = false;
    
    if (!initialized) {
        Serial.println("[ResearchForm] Initializing questions array...");
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
            questions[5 + i] = {feeling_adjectives[i], feeling_adjectives[i], feeling_scale, 4};
        }
        
        // Page 29: Concentration
        questions[29] = {"poziom_skupienia", "Poziom skupienia dzisiaj", concentration_options, 7};
        
        // Page 30: Concert frequency
        questions[30] = {"częstość_koncertów", "Jak często bywasz\nna koncertach?", concert_freq_options, 5};
        
        // Page 31: Music preferences
        questions[31] = {"muzyka_na_co_dzień", "Jakiej muzyki słuchasz\nna co dzień?", music_pref_options, 5};
        
        initialized = true;
        Serial.println("[ResearchForm] Questions array initialized successfully");
    }
    
    if (page >= 1 && page <= 31) {
        Serial.printf("[ResearchForm] Returning question for page %d\n", page);
        return &questions[page];
    }
    Serial.printf("[ResearchForm] Page %d out of range, returning nullptr\n", page);
    return nullptr;
}

const char* ResearchFormPage::get_section_header(int page) {
    if (page >= 5 && page <= 28) {
        return "Jak się dzisiaj czujesz?";
    }
    return nullptr;
}

void ResearchFormPage::on_form_submit() {
    Serial.println("Research Form submitted!");
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
    Serial.println("\n=== RESEARCH FORM DATA ===");
    Serial.println(json);
    Serial.println("==========================\n");
    
    // TODO: Send JSON via HTTP/MQTT
    
    // Navigate back to BEFORE_CONCERT page
    Serial.println("Navigating back to BEFORE_CONCERT");
    navigator.showPage(BEFORE_CONCERT);
}
