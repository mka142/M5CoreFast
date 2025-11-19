#include "TensionMeasurementPage.h"
#include <Arduino.h>
#include <polish_fonts.h>
#include <stdio.h>
#include <cmath>

lv_obj_t* TensionMeasurementPage::bar_obj = nullptr;
lv_obj_t* TensionMeasurementPage::center_line = nullptr;
lv_obj_t* TensionMeasurementPage::side_lines[3] = {nullptr, nullptr, nullptr};
lv_obj_t* TensionMeasurementPage::buffer_label = nullptr;
lv_obj_t* TensionMeasurementPage::scale_labels[6] = {nullptr};
lv_style_t TensionMeasurementPage::style_indic;
float TensionMeasurementPage::current_position = 0.0f;
float TensionMeasurementPage::velocity = 0.0f;
float TensionMeasurementPage::wave_amplitude = 0.0f;
float TensionMeasurementPage::wave_phase = 0.0f;
lv_timer_t* TensionMeasurementPage::momentum_timer = nullptr;

lv_obj_t* TensionMeasurementPage::create() {
    // Create screen - czarne tło
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

    // ==================== VERTICAL BAR (120x200px) - minimalistyczny, płynny, szary ====================
    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_color_hex(0x404040)); // Ciemny szary
    lv_style_set_bg_grad_color(&style_indic, lv_color_hex(0xC0C0C0)); // Jasny szary
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);
    lv_style_set_radius(&style_indic, 0); // No rounding for indicator

    static lv_style_t style_main;
    lv_style_init(&style_main);
    lv_style_set_bg_opa(&style_main, LV_OPA_COVER);
    lv_style_set_bg_color(&style_main, lv_color_hex(0x202020)); // Slightly darker background
    lv_style_set_radius(&style_main, 0); // No rounding for main/background

    lv_obj_t *bar = lv_bar_create(screen);
    // Apply styles to both main and indicator parts to ensure fully flat corners
    lv_obj_add_style(bar, &style_main, LV_PART_MAIN);
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    // Increase width by 20% on each side -> total width *= 1.4
    lv_obj_set_size(bar, 168, 200); // was 120, now ~168
    lv_obj_center(bar);
    // Visual / logical range: 0..100
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    bar_obj = bar;

    // ==================== CYFRY NA ŚRODKU (0,2,4,6,8,10) ====================
    // Create labels for 0,20,40,60,80,100 aligned along the bar
    int label_idx = 0;
    const int bar_h = 200;
    const int pad = 12; // keep labels inside bar bounds
    const int eff_h = bar_h - (pad * 2);
    for (int v = 0; v <= 100; v += 20) {
        // y offset relative to center inside the padded area
        int y_pos = (int)(((100 - v) / 100.0f) * eff_h - (eff_h / 2));

        char label_text[5];
        snprintf(label_text, sizeof(label_text), "%d", v);

        scale_labels[label_idx] = lv_label_create(screen);
        lv_label_set_text(scale_labels[label_idx], label_text);
        lv_obj_set_style_text_color(scale_labels[label_idx], lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(scale_labels[label_idx], &montserrat_20_polish, 0);
        lv_obj_align(scale_labels[label_idx], LV_ALIGN_CENTER, 0, y_pos);

        label_idx++;
    }

    // ==================== ANIMOWANE KRESKI ====================
    // Kreska środkowa - szybsza, idzie do poziomu bar
    static lv_style_t style_center_line;
    lv_style_init(&style_center_line);
    lv_style_set_line_width(&style_center_line, 3);
    lv_style_set_line_color(&style_center_line, lv_color_hex(0xFFFFFF));

    static lv_point_precise_t center_points[2];
    center_points[0].x = 160;  // Center
    center_points[0].y = 120;  // Start na dole
    center_points[1].x = 160;
    center_points[1].y = 120;

    center_line = lv_line_create(screen);
    lv_line_set_points(center_line, center_points, 2);
    lv_obj_add_style(center_line, &style_center_line, 0);

    // Usunięto boczne kreski – zostają tylko strzałki

    // ==================== BUFFER STATUS ====================
    // Buffer label temporarily disabled (user requested it hidden)
    // buffer_label = lv_label_create(screen);
    // lv_label_set_text(buffer_label, "Buffer: 0/200");
    // lv_obj_set_style_text_color(buffer_label, lv_color_hex(COLOR_SECONDARY), 0);
    // lv_obj_set_style_text_font(buffer_label, &montserrat_14_polish, 0);
    // lv_obj_align(buffer_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    // Create momentum timer
    if (momentum_timer == nullptr) {
        momentum_timer = lv_timer_create(momentumTimerCallback, MOMENTUM_INTERVAL, nullptr);
    }

    // numeric readout removed (user requested)

    return screen;
}

void TensionMeasurementPage::handleEncoder(int delta) {
    if (!bar_obj) return;

    // Adaptive movement: base sensitivity scaled by delta magnitude
    int d = delta;
    int absd = abs(d);
    int capped = absd;
    if (capped > ADAPTIVE_DELTA_MAX) capped = ADAPTIVE_DELTA_MAX;
    float adapt = 1.0f + (ADAPTIVE_GAIN * (float)capped);
    float movement = delta * ENCODER_SENSITIVITY * adapt;
    // Cap per-tick movement to avoid huge jumps
    if (movement > MAX_STEP_PER_TICK) movement = MAX_STEP_PER_TICK;
    if (movement < -MAX_STEP_PER_TICK) movement = -MAX_STEP_PER_TICK;

    // Add to velocity for momentum effect
    velocity += movement;

    // Throttled debug (kept minimal)
    static unsigned long last_dbg = 0;
    if (millis() - last_dbg > 1000) {
        Serial.printf("TENSION d=%d mv=%.2f vel=%.2f pos=%.2f\n", delta, movement, velocity, current_position);
        last_dbg = millis();
    }

    // Create wave ripple effect - amplitude proportional to velocity
    wave_amplitude = fabs(velocity) * 0.5f;  // Scale down for visual effect
    if (wave_amplitude > 10.0f) wave_amplitude = 10.0f;  // Cap at 10px

    // Update position immediately
    current_position += movement;

    // Clamp to 0..100 range
    if (current_position < 0.0f) {
        current_position = 0.0f;
        velocity = 0.0f;  // Stop at boundary
    }
    if (current_position > 100.0f) {
        current_position = 100.0f;
        velocity = 0.0f;  // Stop at boundary
    }

    // Immediately reflect the manual adjustment without LVGL animation for responsiveness
    if (bar_obj) {
        int bar_value = (int)(current_position + 0.5f);
        lv_bar_set_value(bar_obj, bar_value, LV_ANIM_OFF);

        // numeric readout removed (user requested)

        // Update scale label colors (0,20,40,60,80,100)
        for (int i = 0; i < 6; i++) {
            int label_value = i * 20; // 0,20,40,60,80,100
            if (bar_value >= label_value) {
                lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0xFFFFFF), 0);
            } else {
                lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0x404040), 0);
            }
        }
    }
}

void TensionMeasurementPage::momentumTimerCallback(lv_timer_t *timer) {
    // Apply momentum decay
    if (fabs(velocity) > VELOCITY_THRESHOLD) {
        velocity *= MOMENTUM_DECAY;
        current_position += velocity;

        // Decay wave amplitude with velocity
        wave_amplitude *= MOMENTUM_DECAY;
        wave_phase += 0.3f;  // Advance wave animation

        // Clamp position
        if (current_position < 0.0f) {
            current_position = 0.0f;
            velocity = 0.0f;
        }
        if (current_position > 100.0f) {
            current_position = 100.0f;
            velocity = 0.0f;
        }

        updateDisplay();
    } else {
        velocity = 0.0f;
        wave_amplitude = 0.0f;
    }
}

void TensionMeasurementPage::updateDisplay() {
    if (!bar_obj) return;

    int bar_value = (int)(current_position + 0.5f);  // Round to nearest int (0..100)
    lv_bar_set_value(bar_obj, bar_value, LV_ANIM_ON);

    // Usunięcie łuku - bar ma zaokrągloną górną krawędź
    // Gradientowa kurtyna: blendowanie cyfr przez pionowy gradient opacity
    // (realizowane przez styl bara LVGL)

    // Blendowanie cyfr przez szarą kurtynę (labels at 0,20,40,60,80,100)
    for (int i = 0; i < 6; i++) {
        int label_value = i * 20;  // 0,20,40,60,80,100
        if (bar_value >= label_value) {
            lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0xFFFFFF), 0);
        } else {
            lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0x404040), 0);
        }
    }

    // No side-arrow animation (removed) — keep UI minimal and stable
}

void TensionMeasurementPage::updateValue(int value) {
    // Legacy method - now using handleEncoder for direct control
    current_position = (float)value;
    if (current_position < 0.0f) current_position = 0.0f;
    if (current_position > 100.0f) current_position = 100.0f;
    updateDisplay();
}

void TensionMeasurementPage::updateBuffer(int count, int max) {
    if (buffer_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Buffer: %d/%d", count, max);
        lv_label_set_text(buffer_label, buf);
    }
}

int TensionMeasurementPage::getCurrentValue() {
    return (int)(current_position + 0.5f);
}

void TensionMeasurementPage::cleanup() {
    if (momentum_timer) {
        lv_timer_del(momentum_timer);
        momentum_timer = nullptr;
    }
    velocity = 0.0f;
    wave_amplitude = 0.0f;
    wave_phase = 0.0f;
}
