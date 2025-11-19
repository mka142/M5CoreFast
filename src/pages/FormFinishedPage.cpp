#include "FormFinishedPage.h"
#include <polish_fonts.h>
#include <ThemeColors.h>
#include <RotatingNote.h>

// Static member initialization
lv_obj_t* FormFinishedPage::screen = nullptr;
lv_obj_t* FormFinishedPage::main_label = nullptr;
lv_obj_t* FormFinishedPage::subtitle_label = nullptr;
RotatingNote* FormFinishedPage::rotating_note = nullptr;

lv_obj_t* FormFinishedPage::create() {
    // Create screen with black background
    screen = lv_obj_create(nullptr);
    lv_obj_set_size(screen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(screen, 0, 0);
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
    
    // Create rotating note (cyan color: 0x42B2C2)
    if (!rotating_note) {
        rotating_note = new RotatingNote();
    }
    rotating_note->create(screen, 0x42B2C2, 0, 10);
    rotating_note->startRotation();
    
    // Main text label (white, 32pt) - positioned below note
    main_label = lv_label_create(screen);
    lv_label_set_text(main_label, "Dziękujemy!");
    lv_obj_set_style_text_color(main_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(main_label, &montserrat_32_polish, 0);
    lv_obj_set_style_text_align(main_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(main_label, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_long_mode(main_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(main_label, 280);
    
    // Subtitle label (gray, 14pt)
    subtitle_label = lv_label_create(screen);
    lv_label_set_text(subtitle_label, "Proszę czekać na dalszą część koncertu");
    lv_obj_set_style_text_color(subtitle_label, lv_color_hex(0x999999), 0);
    lv_obj_set_style_text_font(subtitle_label, &montserrat_14_polish, 0);
    lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(subtitle_label, LV_ALIGN_CENTER, 0, 80);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(subtitle_label, 280);
    
    return screen;
}

void FormFinishedPage::cleanup() {
    if (rotating_note) {
        rotating_note->cleanup();
        delete rotating_note;
        rotating_note = nullptr;
    }
    screen = nullptr;
    main_label = nullptr;
    subtitle_label = nullptr;
}
