#include "AppGuidePage.h"
#include <polish_fonts.h>
#include <RGBAdapter.h>

// Declare external RGB adapter
extern RGBAdapter rgb;

lv_obj_t *AppGuidePage::screen = nullptr;
lv_obj_t *AppGuidePage::scroll_container = nullptr;

lv_obj_t *AppGuidePage::create()
{
    // Create screen with black background
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_BLACK), 0);

    // Create scrollable container
    scroll_container = lv_obj_create(screen);
    lv_obj_set_size(scroll_container, 320, 240);
    lv_obj_align(scroll_container, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(scroll_container, lv_color_hex(COLOR_BLACK), 0);
    lv_obj_set_style_border_width(scroll_container, 0, 0);
    lv_obj_set_scrollbar_mode(scroll_container, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(scroll_container, 15, 0);
    lv_obj_set_flex_flow(scroll_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(scroll_container, 15, 0);

    // Top header: "Drogi Słuchaczu!"
    lv_obj_t *header = lv_label_create(scroll_container);
    lv_label_set_text(header, "Drogi Słuchaczu!");
    lv_obj_set_style_text_color(header, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(header, &montserrat_32_polish, 0);
    lv_obj_set_width(header, 290);

    // Introduction text
    lv_obj_t *intro_text = lv_label_create(scroll_container);
    lv_label_set_text(intro_text,
                      "Przed rozpoczęciem koncertu badawczego, prosimy o zapoznanie się z poniższymi "
                      "wskazówkami, które pomogą Ci w pełni cieszyć się doświadczeniem muzycznym.");
    lv_obj_set_style_text_color(intro_text, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(intro_text, &montserrat_14_polish, 0);
    lv_label_set_long_mode(intro_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(intro_text, 290);

    // Info cards
    create_info_card(scroll_container,
                     LV_SYMBOL_VOLUME_MAX,
                     "Masz telefon?",
                     "Zalecamy wyciszenie telefonu i wyłączenie wibracji, aby w pełni zanurzyć się w doznaniach muzycznych.",
                     0);

    create_info_card(scroll_container,
                     LV_SYMBOL_AUDIO,
                     "Podczas koncertu",
                     "Urządzenie jest Twoim przewodnikiem i rejestratorem danych. Prosimy, miej je cały czas przy sobie!",
                     0);

    create_info_card(scroll_container,
                     LV_SYMBOL_CHARGE,
                     "Napięcie muzyczne",
                     "Podczas trwania koncertu będziesz rejestrować odczucia napięcia muzycznego. Obracaj pokrętło w górę i w dół, aby zmieniać wartość napięcia.",
                     0);

    create_info_card(scroll_container,
                     LV_SYMBOL_WARNING,
                     "Po koncercie",
                     "Prosimy o zwrot urządzenia organizatorom - dziękujemy!",
                     0);

    return screen;
}

void AppGuidePage::firstRender()
{
    // Turn off RGB LEDs for app guide page
    rgb.setColor(0, 0, 0);
}

void AppGuidePage::lastRender()
{
    // No special cleanup needed
}

void AppGuidePage::create_info_card(lv_obj_t *parent, const char *icon, const char *title, const char *description, int y_offset)
{
    // Create card container (slightly brighter than background)
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 290, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x1a1a1a), 0); // Slightly brighter than black
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 12, 0);

    // Icon label (left side, using default LVGL font for symbols)
    lv_obj_t *icon_label = lv_label_create(card);
    lv_label_set_text(icon_label, icon);
    lv_obj_set_style_text_color(icon_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_24, 0); // Default font for symbols
    lv_obj_align(icon_label, LV_ALIGN_TOP_LEFT, 0, 0);

    // Title label (right next to icon)
    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(title_label, &montserrat_20_polish, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, 230); // Width minus icon space
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 40, 2);

    // Description label (below icon and title)
    lv_obj_t *desc_label = lv_label_create(card);
    lv_label_set_text(desc_label, description);
    lv_obj_set_style_text_color(desc_label, lv_color_hex(0x999999), 0); // Gray color for secondary text
    lv_obj_set_style_text_font(desc_label, &montserrat_14_polish, 0);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_label, 266);
    lv_obj_align(desc_label, LV_ALIGN_TOP_LEFT, 0, 35);
}

void AppGuidePage::cleanup()
{
    screen = nullptr;
    scroll_container = nullptr;
}
