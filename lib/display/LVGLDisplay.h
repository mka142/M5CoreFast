#pragma once
#include <lvgl.h>
#include <M5Unified.h>

// Simple LVGL display driver for M5CoreS3
// No abstraction - just what we need

class LVGLDisplay {
public:
    static void init(uint16_t hor_res, uint16_t ver_res);
    static void flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
    static uint32_t tick_cb();
    
private:
    static lv_display_t *display;
};
