/**
 * @file SliderDemoPage.h
 * @brief Vertical fader slider demo page with custom PNG graphics
 * 
 * Professional fader control suitable for National Forum of Music application.
 * Features custom knob graphics, accessible touch controls, and elegant appearance.
 */

#pragma once

#include <lvgl.h>
#include <ThemeColors.h>

/**
 * @class SliderDemoPage
 * @brief Demo page showing vertical fader slider with custom graphics
 * 
 * Creates a professional-looking vertical fader with:
 * - Range: 1-10
 * - Custom PNG knob (199x52 px)
 * - Large touch hitbox (100px width) for accessibility
 * - Discrete scale marks with numeric labels
 * - Drop shadow for 3D effect
 */
class SliderDemoPage {
public:
    /**
     * @brief Create and configure the slider demo page
     * @return Pointer to the created LVGL screen object
     */
    static lv_obj_t* create();
    
    /**
     * @brief Slider value change callback
     * @param e LVGL event object
     * 
     * Updates numeric label and knob position when slider value changes.
     */
    static void slider_event_cb(lv_event_t *e);
    
private:
    static lv_obj_t *value_label;  ///< Label displaying current slider value
};
