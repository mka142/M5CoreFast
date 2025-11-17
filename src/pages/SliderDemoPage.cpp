/**
 * @file SliderDemoPage.cpp
 * @brief Demo page with vertical fader slider using custom PNG graphics
 * 
 * Displays a professional fader control with:
 * - Vertical slider (1-10 range)
 * - Custom knob from fader PNG image (199x52 px)
 * - Scale with tick marks and numeric labels
 * - Large touch hitbox (100px) for accessibility
 */

#include "SliderDemoPage.h"
#include <polish_fonts.h>
#include <images/fader_knob.h>
#include <stdio.h>

// Static member initialization
lv_obj_t* SliderDemoPage::value_label = nullptr;

// Internal state - not exposed outside this file
static lv_obj_t *slider_obj  = nullptr;  // Invisible slider widget (logic only)
static lv_obj_t *knob_obj    = nullptr;  // PNG image for knob
static lv_obj_t *scale_line  = nullptr;  // Vertical scale track
static lv_coord_t last_y     = 0;        // Last touch Y position for delta tracking
static bool touch_active     = false;     // Whether touch is currently active

// Forward declarations
static void update_knob_pos();
static void create_scale(lv_obj_t *parent);
static void touch_event_cb(lv_event_t *e);
static void release_event_cb(lv_event_t *e);

/*----------------------------------------------------------------------------*/
/* Public API                                                                  */
/*----------------------------------------------------------------------------*/

lv_obj_t* SliderDemoPage::create() {
    // Create root screen with dark background
    lv_obj_t *screen = lv_obj_create(nullptr);
    lv_obj_remove_style_all(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x101010), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // Create scale first (sets scale_line for alignment)
    create_scale(screen);

    // Create invisible slider widget for touch logic
    // Full screen width for maximum touch sensitivity
    slider_obj = lv_slider_create(screen);
    lv_obj_remove_style_all(slider_obj);
    lv_obj_set_size(slider_obj, 320, lv_obj_get_height(scale_line));
    lv_obj_align_to(slider_obj, scale_line, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider_obj, 1, 10);
    lv_slider_set_value(slider_obj, 5, LV_ANIM_OFF);
    lv_slider_set_orientation(slider_obj, LV_SLIDER_ORIENTATION_VERTICAL);
    lv_obj_clear_flag(slider_obj, LV_OBJ_FLAG_SCROLLABLE);
    
    // Hide all slider parts (we use custom graphics)
    lv_obj_set_style_bg_opa(slider_obj, LV_OPA_TRANSP, 
                            LV_PART_MAIN | LV_PART_INDICATOR | LV_PART_KNOB);
    lv_obj_set_style_border_opa(slider_obj, LV_OPA_TRANSP, LV_PART_MAIN);
    
    // Add touch events for gesture-based control
    lv_obj_add_event_cb(slider_obj, touch_event_cb, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(slider_obj, touch_event_cb, LV_EVENT_PRESSING, nullptr);
    lv_obj_add_event_cb(slider_obj, release_event_cb, LV_EVENT_RELEASED, nullptr);

    // Value label above scale
    value_label = lv_label_create(screen);
    lv_label_set_text(value_label, "5");
    lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(value_label, &montserrat_24_polish, 0);
    lv_obj_align_to(value_label, scale_line, LV_ALIGN_OUT_TOP_MID, 0, -4);

    // Custom knob from PNG image (199x52 px, ARGB8888 format)
    knob_obj = lv_image_create(screen);
    lv_image_set_src(knob_obj, &fader_knob);
    
    // Drop shadow for 3D effect
    lv_obj_set_style_shadow_width(knob_obj, 8, 0);
    lv_obj_set_style_shadow_color(knob_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_ofs_y(knob_obj, 3, 0);
    lv_obj_set_style_shadow_opa(knob_obj, 160, 0);
    
    // Position knob and bring to front
    lv_obj_update_layout(scale_line);
    update_knob_pos();
    lv_obj_move_foreground(knob_obj);

    // Register value change callback
    lv_obj_add_event_cb(slider_obj, slider_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

    return screen;
}

void SliderDemoPage::slider_event_cb(lv_event_t *e) {
    LV_UNUSED(e);
    if (!slider_obj) return;
    
    int32_t value = lv_slider_get_value(slider_obj);
    
    // Update numeric label
    if (value_label) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", value);
        lv_label_set_text(value_label, buf);
    }
    
    // Update knob position
    update_knob_pos();
}

/*----------------------------------------------------------------------------*/
/* Internal helpers                                                            */
/*----------------------------------------------------------------------------*/

/**
 * @brief Handle touch events for immediate slider response
 * 
 * Responds to PRESSED and PRESSING events to:
 * 1. Track touch delta (movement up/down from initial touch)
 * 2. Move slider proportionally to finger movement
 * 3. Allow control from anywhere on screen
 */
static void touch_event_cb(lv_event_t *e) {
    if (!slider_obj || !scale_line) return;
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_indev_t *indev = lv_indev_active();
    if (!indev) return;
    
    lv_point_t point;
    lv_indev_get_point(indev, &point);
    
    // Get scale dimensions
    const lv_coord_t line_h = lv_obj_get_height(scale_line);
    const int32_t vmin = lv_slider_get_min_value(slider_obj);
    const int32_t vmax = lv_slider_get_max_value(slider_obj);
    
    if (code == LV_EVENT_PRESSED) {
        // Initial touch - store starting position
        last_y = point.y;
        touch_active = true;
    }
    else if (code == LV_EVENT_PRESSING && touch_active) {
        // Calculate delta from last position
        const lv_coord_t delta_y = last_y - point.y;  // Inverted: up = positive
        last_y = point.y;
        
        // Convert pixel delta to value delta
        // Scale sensitivity: 1 pixel = (range / height) values
        const float pixels_per_value = (float)line_h / (float)(vmax - vmin);
        const float value_delta = (float)delta_y / pixels_per_value;
        
        // Get current value and apply delta
        const int32_t current = lv_slider_get_value(slider_obj);
        const int32_t new_value = current + (int32_t)(value_delta + 0.5f);
        
        // Clamp to valid range
        const int32_t clamped = (new_value < vmin) ? vmin : (new_value > vmax) ? vmax : new_value;
        
        // Update slider (will trigger VALUE_CHANGED event)
        if (clamped != current) {
            lv_slider_set_value(slider_obj, clamped, LV_ANIM_OFF);
        }
    }
}

/**
 * @brief Handle touch release event
 */
static void release_event_cb(lv_event_t *e) {
    LV_UNUSED(e);
    touch_active = false;
}

/**
 * @brief Update knob position based on slider value
 * 
 * Maps slider value (1-10) to vertical position along scale.
 * Knob is centered horizontally on scale and positioned vertically
 * so its center aligns with the corresponding scale position.
 */
static void update_knob_pos() {
    if (!slider_obj || !knob_obj || !scale_line) return;

    // Get current slider value and range
    const int32_t v    = lv_slider_get_value(slider_obj);
    const int32_t vmin = lv_slider_get_min_value(slider_obj);
    const int32_t vmax = lv_slider_get_max_value(slider_obj);

    // Get scale dimensions and position
    const lv_coord_t line_x = lv_obj_get_x(scale_line);
    const lv_coord_t line_y = lv_obj_get_y(scale_line);
    const lv_coord_t line_h = lv_obj_get_height(scale_line);

    // Calculate vertical position (0.0 = bottom, 1.0 = top)
    const float ratio = (float)(v - vmin) / (float)(vmax - vmin);
    const lv_coord_t bottom   = line_y + line_h;
    const lv_coord_t center_y = bottom - (lv_coord_t)(ratio * line_h);

    // Center knob horizontally on scale, vertically on value position
    const lv_coord_t knob_w = lv_obj_get_width(knob_obj);
    const lv_coord_t knob_h = lv_obj_get_height(knob_obj);
    const lv_coord_t knob_x = line_x + lv_obj_get_width(scale_line) / 2 - knob_w / 2;
    const lv_coord_t knob_y = center_y - knob_h / 2;

    lv_obj_set_pos(knob_obj, knob_x, knob_y);
}

/**
 * @brief Create vertical scale with track, tick marks and labels
 * 
 * Creates:
 * - Vertical track (6x160 px, centered on screen)
 * - 10 level marks (small ticks on both sides)
 * - Numeric labels 1-10 (only on right side)
 * 
 * @param parent Parent LVGL object to attach scale to
 */
static void create_scale(lv_obj_t *parent) {
    constexpr lv_coord_t scale_height = 160;
    constexpr lv_coord_t line_width   = 6;

    // Create vertical track centered on screen
    scale_line = lv_obj_create(parent);
    lv_obj_remove_style_all(scale_line);
    lv_obj_set_size(scale_line, line_width, scale_height);
    lv_obj_set_style_bg_color(scale_line, lv_color_hex(0x505050), 0);
    lv_obj_set_style_bg_opa(scale_line, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(scale_line, 3, 0);
    lv_obj_center(scale_line);

    // Force layout update to get valid coordinates
    lv_obj_update_layout(scale_line);
    const lv_coord_t line_x = lv_obj_get_x(scale_line);
    const lv_coord_t line_y = lv_obj_get_y(scale_line);
    const lv_coord_t bottom = line_y + scale_height;

    constexpr int levels = 10;
    constexpr int step   = scale_height / levels;

    // Create tick marks and labels for each level (1-10)
    for (int i = 1; i <= levels; ++i) {
        const lv_coord_t tick_y = bottom - i * step;

        // Left tick mark (small discrete line)
        lv_obj_t *tick_left = lv_obj_create(parent);
        lv_obj_remove_style_all(tick_left);
        lv_obj_set_size(tick_left, 4, 1);
        lv_obj_set_style_bg_color(tick_left, lv_color_hex(0x707070), 0);
        lv_obj_set_style_bg_opa(tick_left, LV_OPA_COVER, 0);
        lv_obj_set_pos(tick_left, line_x - 6, tick_y);

        // Right tick mark (mirror of left)
        lv_obj_t *tick_right = lv_obj_create(parent);
        lv_obj_remove_style_all(tick_right);
        lv_obj_set_size(tick_right, 4, 1);
        lv_obj_set_style_bg_color(tick_right, lv_color_hex(0x707070), 0);
        lv_obj_set_style_bg_opa(tick_right, LV_OPA_COVER, 0);
        lv_obj_set_pos(tick_right, line_x + line_width + 2, tick_y);

        // Numeric label (only on right side)
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", i);
        
        lv_obj_t *lbl = lv_label_create(parent);
        lv_label_set_text(lbl, buf);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_text_font(lbl, &montserrat_12_polish, 0);
        lv_obj_set_pos(lbl, line_x + line_width + 8, tick_y - 6);
    }
}