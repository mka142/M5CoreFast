#include "SliderDemoPage.h"
#include <polish_fonts.h>
#include <PageID.h>
#include <PageNavigator.h>
#include <Arduino.h>
#include <RGBAdapter.h>

// Declare external image
LV_IMG_DECLARE(note);

// External references (defined in main.cpp)
extern PageNavigator navigator;
extern RGBAdapter rgb;

// Static member initialization
lv_obj_t *SliderDemoPage::screen = nullptr;
lv_obj_t *SliderDemoPage::button = nullptr;

lv_obj_t *SliderDemoPage::create()
{
    // Create screen with gradient background (0x9261D5 -> 0x42B2C2)
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x9261D5), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x42B2C2), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_HOR, 0);

    // Create title label
    lv_obj_t *title_label = lv_label_create(screen);
    lv_label_set_text(title_label, "Przetestuj suwak");
    lv_obj_set_style_text_color(title_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(title_label, &montserrat_48_polish, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, -20);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(title_label, 280);

    // Create gradient button at bottom
    button = lv_button_create(screen);
    lv_obj_set_size(button, 300, 50);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_radius(button, 25, 0); // Rounded corners

    // Button gradient (0xFF4B4B -> 0xFA6737)
    lv_obj_set_style_bg_color(button, lv_color_hex(0xFF4B4B), 0);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0xFA6737), 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);

    // Button label
    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, "Przetestuj");
    lv_obj_set_style_text_color(button_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(button_label, &montserrat_20_polish, 0);
    lv_obj_center(button_label);

    // Add button click event handler
    lv_obj_add_event_cb(button, on_test_button_clicked, LV_EVENT_CLICKED, NULL);

    return screen;
}

void SliderDemoPage::firstRender()
{
    // Turn off RGB LEDs
    rgb.setColor(0, 0, 0);
}

void SliderDemoPage::lastRender()
{
    // Turn off RGB LEDs when leaving the page
    rgb.setColor(0, 0, 0);
}

void SliderDemoPage::on_test_button_clicked(lv_event_t *e)
{
    Serial.println("Test button clicked - navigating to SLIDER_DEMO__MEASUREMENT");
    navigator.showPage(SLIDER_DEMO__MEASUREMENT);
}

void SliderDemoPage::cleanup()
{
    screen = nullptr;
    button = nullptr;
}
