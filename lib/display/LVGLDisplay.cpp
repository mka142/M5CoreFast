#include "LVGLDisplay.h"
#include <esp_timer.h>

lv_display_t* LVGLDisplay::display = nullptr;

void LVGLDisplay::init(uint16_t hor_res, uint16_t ver_res) {
    lv_init();
    
    // Set tick callback
    lv_tick_set_cb(tick_cb);
    
    // Create display
    display = lv_display_create(hor_res, ver_res);
    lv_display_set_flush_cb(display, flush_cb);
    
    // Allocate buffer - single buffer, partial rendering
    // Buffer size: HOR_RES * 15 lines (from example)
    static lv_color_t buf1[320 * 15];
    lv_display_set_buffers(display, buf1, nullptr, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void LVGLDisplay::flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    // Swap RGB565 bytes for M5 display
    lv_draw_sw_rgb565_swap(px_map, w * h);
    
    // Push to display using DMA
    M5.Display.pushImageDMA<uint16_t>(area->x1, area->y1, w, h, (uint16_t *)px_map);
    
    // Signal LVGL that flushing is done
    lv_disp_flush_ready(disp);
}

uint32_t LVGLDisplay::tick_cb() {
    return (esp_timer_get_time() / 1000LL);
}
