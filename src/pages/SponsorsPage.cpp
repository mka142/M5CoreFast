#include "SponsorsPage.h"
#include <SponsorCarousel.h>
#include <ThemeColors.h>

// Include sponsor logo images (will be created below)
#include <images/sponsor_logos.h>

lv_obj_t* SponsorsPage::screen = nullptr;

lv_obj_t* SponsorsPage::create() {
    // Create screen with white background for sponsor showcase
    screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), 0);
    
    // Title
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Sponsorzy");
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    // Background panel for logo area
    lv_obj_t *logo_bg = lv_obj_create(screen);
    lv_obj_set_size(logo_bg, 300, 200);
    lv_obj_set_pos(logo_bg, 10, 30);
    lv_obj_set_style_bg_color(logo_bg, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(logo_bg, 0, 0);
    lv_obj_set_style_pad_all(logo_bg, 0, 0);
    lv_obj_set_style_radius(logo_bg, 8, 0);
    
    // Initialize carousel
    SponsorCarousel::init(logo_bg, 0, 0, 300, 200);
    
    // Add all sponsor logos
    SponsorCarousel::addLogo(&logo_wca);
    SponsorCarousel::addLogo(&logo_wroclaw);
    SponsorCarousel::addLogo(&logo_kannm);
    SponsorCarousel::addLogo(&logo_knakitm);
    SponsorCarousel::addLogo(&logo_amkl);
    SponsorCarousel::addLogo(&logo_sknm);
    
    // Start with shuffled order, then loop
    SponsorCarousel::start(3000, true);
    
    return screen;
}

void SponsorsPage::cleanup() {
    SponsorCarousel::cleanup();
}
