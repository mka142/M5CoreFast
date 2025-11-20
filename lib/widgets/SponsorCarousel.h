#pragma once
#include <lvgl.h>
#include <vector>

/**
 * @brief Sponsor Carousel Widget
 * 
 * Displays sponsor logos one at a time with smooth crossfade transitions.
 * Supports smart batch randomization (all logos shown once before repeat).
 * 
 * Usage:
 *   SponsorCarousel::init(screen, x, y, width, height);
 *   SponsorCarousel::addLogo(img_desc);
 *   SponsorCarousel::start(3000, true);  // 3s per logo, randomized
 */

class SponsorCarousel {
public:
    /**
     * @brief Initialize carousel widget
     * @param parent Parent LVGL object (usually screen)
     * @param x X position
     * @param y Y position  
     * @param width Widget width
     * @param height Widget height
     */
    static void init(lv_obj_t *parent, int x, int y, int width, int height);
    
    /**
     * @brief Add logo image descriptor
     * @param img_desc Pointer to LVGL image descriptor (C array)
     */
    static void addLogo(const lv_image_dsc_t *img_desc);
    
    /**
     * @brief Start carousel animation
     * @param duration_ms Display duration per logo (ms)
     * @param random If true, randomize order (smart batch algorithm)
     */
    static void start(uint32_t duration_ms = 3000, bool random = false);
    
    /**
     * @brief Stop carousel animation
     */
    static void stop();
    
    /**
     * @brief Clean up resources
     */
    static void cleanup();
    
    /**
     * @brief Check if running
     */
    static bool isRunning() { return timer != nullptr; }

    // Expose container for external event handling (e.g., touch-to-exit)
    static lv_obj_t* getContainer() { return container; }
    
private:
    // Mode-specific implementations
    static void start_carousel_mode();
    static void start_mosaic_mode();
    static void start_zoom_spotlight_mode();
    static void start_wave_mode();
    
    static void cleanup_carousel_mode();
    static void cleanup_mosaic_mode();
    static void cleanup_zoom_spotlight_mode();
    static void cleanup_wave_mode();
    
    // Carousel mode helpers
    static void timer_cb(lv_timer_t *timer);
    static void fade_anim_cb(void *var, int32_t value);
    static void show_next_logo();
    static void shuffle_batch();
    
    static lv_obj_t *container;
    static lv_obj_t *parent_obj;
    static int pos_x, pos_y, width, height;
    static std::vector<const lv_image_dsc_t*> logos;
    static lv_obj_t *img_current;
    static lv_obj_t *img_next;
    static lv_timer_t *timer;
    static lv_anim_t fade_out_anim;
    static lv_anim_t fade_in_anim;
    static bool random_mode;
    static uint32_t display_duration;
    static std::vector<int> batch_order;
    static int batch_index;
    static int last_logo_index;
    static int current_index;
    static bool animating;
};
