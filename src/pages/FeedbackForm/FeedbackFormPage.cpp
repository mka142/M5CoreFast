#include "FeedbackFormPage.h"
#include <polish_fonts.h>
#include <Arduino.h>
#include <PageID.h>
#include <PageNavigator.h>
#include <ThemeColors.h>

// External page navigator reference (defined in main.cpp)
extern PageNavigator navigator;

// Static instance
FeedbackFormPage* FeedbackFormPage::instance = nullptr;

// ==================== QUESTION DEFINITIONS ====================

// Overall experience options
static const char *overall_experience_options[] = {
    "bardzo pozytywnie",
    "pozytywnie",
    "neutralnie",
    "raczej negatywnie",
    "negatywnie"
};

// Emotional impact adjectives (8 items)
static const char *emotional_impact_adjectives[] = {
    "jestem zrelaksowany/a",
    "jestem pobudzony/a",
    "jestem zainspirowany/a",
    "jestem zmęczony/a",
    "jestem usatysfakcjonowany/a",
    "jestem skoncentrowany/a",
    "jestem poruszony/a",
    "jestem spokojny/a"
};

// Emotional impact scale
static const char *emotional_impact_scale[] = {
    "wcale",
    "słabo",
    "średnio",
    "mocno",
    "bardzo mocno"
};

// Tension slider experience options
static const char *tension_slider_options[] = {
    "bardzo łatwe\ni intuicyjne",
    "łatwe",
    "w miarę łatwe",
    "trudne",
    "bardzo trudne",
    "nie używałem/am\nsuwaka"
};

// Concert length options
static const char *concert_length_options[] = {
    "za krótki",
    "w sam raz",
    "za długi"
};

// Recommendation options
static const char *recommendation_options[] = {
    "zdecydowanie tak",
    "raczej tak",
    "nie wiem",
    "raczej nie",
    "zdecydowanie nie"
};

// ==================== PUBLIC INTERFACE ====================

lv_obj_t* FeedbackFormPage::create() {
    if (!instance) {
        instance = new FeedbackFormPage();
    }
    return instance->create_base(13);  // 13 total pages (0=intro, 1-12=questions)
}

void FeedbackFormPage::cleanup() {
    if (instance) {
        instance->cleanup_base();
        delete instance;
        instance = nullptr;
    }
}

// ==================== PROTECTED IMPLEMENTATIONS ====================

const char* FeedbackFormPage::get_intro_title() {
    return "Formularz Opinii";
}

const char* FeedbackFormPage::get_intro_text() {
    return "Dziękujemy za uczestnictwo w koncercie badawczym!\n\n"
           "Twoja opinia jest dla nas bardzo ważna. Prosimy o wypełnienie krótkiej ankiety, "
           "która pomoże nam lepiej zrozumieć Twoje wrażenia z dzisiejszego koncertu.";
}

void FeedbackFormPage::create_intro_screen() {
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

Question* FeedbackFormPage::get_question(int page) {
    static Question questions[13];
    static bool initialized = false;
    
    if (!initialized) {
        // Page 1: Overall experience
        questions[1] = {"overallExperience", "Jak ogólnie oceniasz\ndzisiejszy koncert\nbadawczy?", overall_experience_options, 5};
        
        // Pages 2-9: Emotional impact (8 questions)
        for (int i = 0; i < 8; i++) {
            questions[2 + i] = {emotional_impact_adjectives[i], emotional_impact_adjectives[i], emotional_impact_scale, 5};
        }
        
        // Page 10: Tension slider experience
        questions[10] = {"tensionSliderExperience", "Jak oceniasz obsługę\nurządzenia do pomiaru\nnapięcia?", tension_slider_options, 6};
        
        // Page 11: Concert length
        questions[11] = {"concertLength", "Jak oceniasz długość\nkoncertu badawczego?", concert_length_options, 3};
        
        // Page 12: Recommendation
        questions[12] = {"recommendation", "Czy poleciłbyś ten\nkoncert znajomym?", recommendation_options, 5};
        
        initialized = true;
    }
    
    if (page >= 1 && page <= 12) {
        return &questions[page];
    }
    return nullptr;
}

const char* FeedbackFormPage::get_section_header(int page) {
    if (page >= 2 && page <= 9) {
        return "W jakim stopniu koncert wpłynął na Twoje samopoczucie?";
    }
    return nullptr;
}

void FeedbackFormPage::on_form_submit() {
    Serial.println("Feedback Form submitted!");
    Serial.println("Creating JSON from answers...");
    
    // Build JSON string
    String json = "{\n";
    bool first_item = true;
    
    // Overall experience (page 1)
    if (answers[1] >= 0) {
        Question *q = get_question(1);
        if (q) {
            json += "  \"" + String(q->key) + "\": \"" + String(q->options[answers[1]]) + "\"";
            first_item = false;
        }
    }
    
    // Emotional impact questions (pages 2-9) - nested object
    json += ",\n  \"emotionalImpact\": {\n";
    bool first_emotion = true;
    for (int page = 2; page <= 9; page++) {
        if (answers[page] >= 0) {
            Question *q = get_question(page);
            if (q) {
                if (!first_emotion) json += ",\n";
                json += "    \"" + String(q->key) + "\": \"" + String(q->options[answers[page]]) + "\"";
                first_emotion = false;
            }
        }
    }
    json += "\n  }";
    
    // Remaining questions (pages 10-12)
    for (int page = 10; page <= 12; page++) {
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
    Serial.println("\n=== FEEDBACK FORM DATA ===");
    Serial.println(json);
    Serial.println("==========================\n");
    
    // TODO: Send JSON via HTTP/MQTT to different endpoint than research form
    
    // Navigate to form finished page
    Serial.println("Navigating to END_OF_CONCERT__FORM_FINISHED");
    navigator.showPage(END_OF_CONCERT__FORM_FINISHED);
}
