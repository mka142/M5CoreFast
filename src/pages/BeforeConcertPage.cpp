#include "BeforeConcertPage.h"
#include <polish_fonts.h>

// Declare external image
LV_IMG_DECLARE(note);

// Static member initialization
lv_obj_t* BeforeConcertPage::screen = nullptr;
lv_obj_t* BeforeConcertPage::note_img = nullptr;
lv_obj_t* BeforeConcertPage::header_label = nullptr;
lv_obj_t* BeforeConcertPage::button = nullptr;
int BeforeConcertPage::rotation_step = 0;
int BeforeConcertPage::current_rotation = 0;  // Start at 0°
int BeforeConcertPage::text_index = 0;
bool BeforeConcertPage::text_scaling_out = false;
lv_anim_t BeforeConcertPage::text_anim;

// Header texts array
const char* BeforeConcertPage::header_texts[] = {
    "Co czują Wrocławianie?",
    "Jakie są ich emocje?",
    "Czy są szczęśliwi?",
    "Czy może są smutni?",
    "Jakiej słuchają muzyki?",
    "Jakie mają marzenia?",
    "Czy muzyka ich łączy?",
    "Kim są Wrocławianie?",
    "Co ich motywuje?",
    "Jakie mają pasje?",
    "Czy są twórczy?"
};
const int BeforeConcertPage::header_texts_count = 11;

// Rotation increments: always move clockwise (right)
// Original sequence was: 30°, 330°, 60°, 300°, 90°, 270°, 120°, 240°, 360°(0°)
// To go continuously clockwise, we calculate the shortest forward path:
// 0→30: +30, 30→330: +300, 330→60: +90 (via 360), 60→300: +240, 300→90: +150 (via 360)
// 90→270: +180, 270→120: +210 (via 360), 120→240: +120, 240→360: +120
const int BeforeConcertPage::rotation_increments[] = {
    300,   // +30° (0.1° units)
    3300,  // +330°
    600,   // +60° (360°→420°, displays as 60°)
    3000,  // +300° (60°→360°)
    900,  // +900° (360°→450°, displays as 90°)
    2700,  // +270° (90°→360°)
    1200,  // +120° (360°→480°, displays as 120°)
    1200,  // +120° (120°→240°)
    1200   // +120° (240°→360°/0°)
};
const int BeforeConcertPage::rotation_increments_count = 9;

void BeforeConcertPage::text_scale_anim_cb(void *var, int32_t value) {
    lv_obj_t *label = (lv_obj_t *)var;
    lv_obj_set_style_opa(label, value, 0);  // Use opacity instead of scale
}

lv_obj_t* BeforeConcertPage::create() {
    // Create screen with gradient background (0x9261D5 -> 0x42B2C2)
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x9261D5), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x42B2C2), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_HOR, 0);
    
    // Create note image (96x130) with rotation support
    note_img = lv_image_create(screen);
    lv_image_set_src(note_img, &note);
    lv_obj_align(note_img, LV_ALIGN_TOP_MID, 0, 10);
    lv_image_set_pivot(note_img, 48, 65);  // Set pivot to center (half of 96x130)
    lv_image_set_scale(note_img, 230);  // Scale down to 90% (256 = 100%)
    
    // Create header label with first text - positioned lower
    header_label = lv_label_create(screen);
    lv_label_set_text(header_label, header_texts[0]);
    lv_obj_set_style_text_color(header_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(header_label, &montserrat_24_polish, 0);
    lv_obj_set_style_text_align(header_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(header_label, LV_ALIGN_CENTER, 0, 30);  // Moved lower (was 0)
    lv_label_set_long_mode(header_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(header_label, 280);
    
    // Create gradient button at bottom
    button = lv_button_create(screen);
    lv_obj_set_size(button, 300, 50);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_radius(button, 25, 0);  // Rounded corners
    
    // Button gradient (0xFF4B4B -> 0xFA6737)
    lv_obj_set_style_bg_color(button, lv_color_hex(0xFF4B4B), 0);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0xFA6737), 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    
    // Button label
    lv_obj_t *button_label = lv_label_create(button);
    lv_label_set_text(button_label, "Wypełnij formularz");
    lv_obj_set_style_text_color(button_label, lv_color_hex(COLOR_WHITE), 0);
    lv_obj_set_style_text_font(button_label, &montserrat_20_polish, 0);
    lv_obj_center(button_label);
    
    // Start rotation animation timer (1500ms per step for slower rotation)
    lv_timer_create(update_rotation, 2000, NULL);
    
    // Start text cycling timer (4000ms per text)
    lv_timer_create(update_text, 4000, NULL);
    
    return screen;
}

// Callback for smooth rotation animation
static void set_angle(void *img, int32_t v) {
    lv_image_set_rotation((lv_obj_t *)img, v);
}

void BeforeConcertPage::update_rotation(lv_timer_t *timer) {
    if (!note_img) return;
    
    // Add increment to current rotation (always moves clockwise)
    int increment = rotation_increments[rotation_step];
    int target_rotation = current_rotation + increment;
    
    // Animate rotation smoothly using lv_image_set_rotation
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, note_img);
    lv_anim_set_exec_cb(&anim, set_angle);
    lv_anim_set_values(&anim, current_rotation, target_rotation);
    lv_anim_set_time(&anim, 1200);  // 1.2s smooth animation
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
    
    // Update current rotation (LVGL handles normalization internally)
    current_rotation = target_rotation;
    
    // Move to next rotation step
    rotation_step = (rotation_step + 1) % rotation_increments_count;
}

void BeforeConcertPage::update_text(lv_timer_t *timer) {
    if (!header_label) return;
    
    if (!text_scaling_out) {
        // Fade out animation
        text_scaling_out = true;
        
        lv_anim_init(&text_anim);
        lv_anim_set_var(&text_anim, header_label);
        lv_anim_set_exec_cb(&text_anim, (lv_anim_exec_xcb_t)text_scale_anim_cb);
        lv_anim_set_values(&text_anim, LV_OPA_COVER, LV_OPA_TRANSP);  // Fade from opaque to transparent
        lv_anim_set_time(&text_anim, 400);
        lv_anim_set_path_cb(&text_anim, lv_anim_path_ease_in);
        lv_anim_set_ready_cb(&text_anim, [](lv_anim_t *a) {
            // Change text when fully faded out
            text_index = (text_index + 1) % header_texts_count;
            lv_label_set_text(header_label, header_texts[text_index]);
            
            // Fade in animation
            lv_anim_t anim_in;
            lv_anim_init(&anim_in);
            lv_anim_set_var(&anim_in, header_label);
            lv_anim_set_exec_cb(&anim_in, (lv_anim_exec_xcb_t)text_scale_anim_cb);
            lv_anim_set_values(&anim_in, LV_OPA_TRANSP, LV_OPA_COVER);  // Fade from transparent to opaque
            lv_anim_set_time(&anim_in, 400);
            lv_anim_set_path_cb(&anim_in, lv_anim_path_ease_out);
            lv_anim_set_ready_cb(&anim_in, [](lv_anim_t *a) {
                text_scaling_out = false;
            });
            lv_anim_start(&anim_in);
        });
        lv_anim_start(&text_anim);
    }
}

void BeforeConcertPage::cleanup() {
    rotation_step = 0;
    current_rotation = 0;
    text_index = 0;
    text_scaling_out = false;
    screen = nullptr;
    note_img = nullptr;
    header_label = nullptr;
    button = nullptr;
}

