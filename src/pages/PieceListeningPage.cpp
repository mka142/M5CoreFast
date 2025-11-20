#include "PieceListeningPage.h"

// Declare external image
LV_IMG_DECLARE(note);

lv_obj_t* PieceListeningPage::create() {
    // Create screen with black background
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

    // Create image object for note icon
    lv_obj_t *note_img = lv_img_create(screen);
    lv_img_set_src(note_img, &note);
    
    // Set dark gray color filter
    lv_obj_set_style_img_recolor(note_img, lv_color_hex(0x404040), 0);  // Dark gray
    lv_obj_set_style_img_recolor_opa(note_img, LV_OPA_COVER, 0);
    
    // Center the image
    lv_obj_align(note_img, LV_ALIGN_CENTER, 0, 0);

    return screen;
}
