#include "TensionMeasurementPage.h"
#include <Arduino.h>
#include <polish_fonts.h>
#include <stdio.h>
#include <cmath>
#include <RGBAdapter.h>

// Access the global RGB adapter instance declared in main.cpp
extern RGBAdapter rgb;

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
float TensionMeasurementPage::breakaway_accumulator = 0.0f;
float TensionMeasurementPage::input_activity = 0.0f;
lv_obj_t* TensionMeasurementPage::recording_led = nullptr;
lv_timer_t* TensionMeasurementPage::recording_led_timer = nullptr;
float TensionMeasurementPage::recording_phase = 0.0f;
lv_obj_t* TensionMeasurementPage::recording_label = nullptr;
lv_obj_t* TensionMeasurementPage::encoder_tooth_parts[3] = {nullptr, nullptr, nullptr};
lv_obj_t* TensionMeasurementPage::encoder_tooth_reflection = nullptr;
lv_obj_t* TensionMeasurementPage::encoder_arrow_up_parts[TensionMeasurementPage::ARROW_DOT_COUNT] = {
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
};
lv_obj_t* TensionMeasurementPage::encoder_arrow_down_parts[TensionMeasurementPage::ARROW_DOT_COUNT] = {
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
    nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
};
lv_timer_t* TensionMeasurementPage::encoder_arrow_timer = nullptr;
float TensionMeasurementPage::encoder_arrow_up_activity = 0.0f;
float TensionMeasurementPage::encoder_arrow_down_activity = 0.0f;
float TensionMeasurementPage::encoder_arrow_phase = 0.0f;
float TensionMeasurementPage::encoder_arrow_progress_up = 0.0f;
float TensionMeasurementPage::encoder_arrow_progress_down = 0.0f;
float TensionMeasurementPage::encoder_arrow_target_idx = 0.0f;
int TensionMeasurementPage::encoder_arrow_dir = 0;
int TensionMeasurementPage::encoder_arrow_hold_frames = 0;
int TensionMeasurementPage::encoder_arrow_last_tail_len = 0;
// Top-left knob removed; no related static objects

lv_obj_t* TensionMeasurementPage::create() {
    // Create screen - czarne tło
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

    // ==================== VERTICAL BAR (120x200px) - minimalistyczny, płynny, szary ====================
    static lv_style_t style_indic;
    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    // Use a single flat indicator color (disable gradient for clearer weight perception)
    lv_style_set_bg_color(&style_indic, lv_color_hex(0x505050)); // Flat mid gray
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
    lv_obj_set_size(bar, 168, 200); // width x height (visual unchanged)
    lv_obj_center(bar);
    // Visual / logical range: use 0..300 for high internal resolution
    lv_bar_set_range(bar, 0, 300);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    bar_obj = bar;
    // Create labels for 0,20,40,60,80,100 aligned along the bar
    int label_idx = 0;
    const int bar_h = 200;
    const int pad = 12; // keep labels inside bar bounds
    const int eff_h = bar_h - (pad * 2);
    // Labels for 0,40,80,120,160,200 to match 0..200 range (6 labels)
           // Labels for 0,60,120,180,240,300 to match 0..300 range (6 labels), displayed as 0..10
           for (int v = 0; v <= 300; v += 60) {
        // y offset relative to center inside the padded area
            int y_pos = (int)(((300 - v) / 300.0f) * eff_h - (eff_h / 2));

        char label_text[8];
        // Map 0..200 -> 0..10 for displayed labels (0,2,4,6,8,10)
            // Map 0..300 -> 0..10 for displayed labels (0,2,4,6,8,10)
            int disp = v / 30; // 0..10
        snprintf(label_text, sizeof(label_text), "%d", disp);

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

    // Create recording LED in bottom-left corner and start pulsing timer
    if (recording_led == nullptr) {
        recording_led = lv_obj_create(screen);
        // Reduce size by 50% (was 12x12)
        lv_obj_set_size(recording_led, 6, 6);
        lv_obj_set_style_radius(recording_led, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(recording_led, 0, 0);
        lv_obj_set_style_bg_color(recording_led, lv_color_hex(0xFF0000), 0);
        // start dimmed; the timer will pulse opacity
        lv_obj_set_style_bg_opa(recording_led, LV_OPA_30, 0);
        lv_obj_align(recording_led, LV_ALIGN_BOTTOM_LEFT, 8, -8);
        recording_phase = 0.0f;
        recording_led_timer = lv_timer_create(recordingLedTimer, 60, nullptr);
        // Create a short label 'zapis' to the right and slightly lower than the LED
        recording_label = lv_label_create(screen);
        lv_label_set_text(recording_label, "zapis");
        lv_obj_set_style_text_color(recording_label, lv_color_hex(0xB0B0B0), 0);
        lv_obj_set_style_text_font(recording_label, &montserrat_12_polish, 0);
        // Align to the right of the LED and slightly higher so the label sits nearer the LED
        lv_obj_align_to(recording_label, recording_led, LV_ALIGN_OUT_RIGHT_MID, 6, -1);

        // (long label removed) keep short 'zapis' label only
    }

    // Top-left illustrative knob removed per user request; only keep recording LED + label

    // Create a small scalloped encoder tooth at the left-middle edge to suggest a protruding
    // mechanical encoder. This uses three overlapping circle pieces so we don't need an image.
    if (encoder_tooth_parts[0] == nullptr) {
        const int part_size = 8;
        // reduce spacing and start higher so the scallop sits nearer the top-left edge
        const int spacing = 5;
        for (int i = 0; i < 3; ++i) {
            encoder_tooth_parts[i] = lv_obj_create(screen);
            lv_obj_set_size(encoder_tooth_parts[i], part_size, part_size);
            lv_obj_set_style_radius(encoder_tooth_parts[i], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(encoder_tooth_parts[i], lv_color_hex(0x303030), 0);
            lv_obj_set_style_border_width(encoder_tooth_parts[i], 0, 0);
            // Align a bit higher than center and protrude more from the bezel
            lv_obj_align(encoder_tooth_parts[i], LV_ALIGN_LEFT_MID, -8, -10 + i * spacing);
        }
        // small subtle reflection/highlight near the top-left edge adjacent to the tooth
        if (encoder_tooth_reflection == nullptr) {
            encoder_tooth_reflection = lv_obj_create(screen);
            lv_obj_set_size(encoder_tooth_reflection, 6, 6);
            lv_obj_set_style_radius(encoder_tooth_reflection, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(encoder_tooth_reflection, lv_color_hex(0xB0B0B0), 0);
            lv_obj_set_style_bg_opa(encoder_tooth_reflection, LV_OPA_40, 0);
            // place it slightly above-left of the top-most tooth piece
            lv_obj_align_to(encoder_tooth_reflection, encoder_tooth_parts[0], LV_ALIGN_OUT_TOP_LEFT, -2, -2);
        }

        // Create vertical micro-LED dot strips (6 per side) aligned along the bar
        // Make dots smaller for less visual dominance (30% smaller -> 6 * 0.7 ≈ 4)
        const int dot_size = 4;
        const int DOT_COUNT = TensionMeasurementPage::ARROW_DOT_COUNT;
        // use the same bar height/padding used for labels above
        const int dot_pad = 12; // keep inside bar bounds
        const int dot_eff_h = bar_h - (dot_pad * 2);
        const float step = (float)dot_eff_h / (float)(DOT_COUNT - 1);
        const float top_offset = -((float)dot_eff_h * 0.5f);
        for (int p = 0; p < DOT_COUNT; ++p) {
            int y_off = (int)roundf(top_offset + (float)p * step);
            if (encoder_arrow_up_parts[p] == nullptr) {
                encoder_arrow_up_parts[p] = lv_obj_create(screen);
                lv_obj_set_size(encoder_arrow_up_parts[p], dot_size, dot_size);
                lv_obj_set_style_radius(encoder_arrow_up_parts[p], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(encoder_arrow_up_parts[p], 0, 0);
                lv_obj_set_style_bg_color(encoder_arrow_up_parts[p], lv_color_hex(0x303030), 0);
                lv_obj_set_style_bg_opa(encoder_arrow_up_parts[p], LV_OPA_TRANSP, 0);
                // left side, align to bar left
                if (bar_obj)
                    lv_obj_align_to(encoder_arrow_up_parts[p], bar_obj, LV_ALIGN_OUT_LEFT_MID, -12, y_off);
                else
                    lv_obj_align(encoder_arrow_up_parts[p], LV_ALIGN_LEFT_MID, -12, y_off);
            }
            if (encoder_arrow_down_parts[p] == nullptr) {
                encoder_arrow_down_parts[p] = lv_obj_create(screen);
                lv_obj_set_size(encoder_arrow_down_parts[p], dot_size, dot_size);
                lv_obj_set_style_radius(encoder_arrow_down_parts[p], LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(encoder_arrow_down_parts[p], 0, 0);
                lv_obj_set_style_bg_color(encoder_arrow_down_parts[p], lv_color_hex(0x303030), 0);
                lv_obj_set_style_bg_opa(encoder_arrow_down_parts[p], LV_OPA_TRANSP, 0);
                // right side, align to bar right
                if (bar_obj)
                    lv_obj_align_to(encoder_arrow_down_parts[p], bar_obj, LV_ALIGN_OUT_RIGHT_MID, 12, y_off);
                else
                    lv_obj_align(encoder_arrow_down_parts[p], LV_ALIGN_RIGHT_MID, 12, y_off);
            }
        }

        if (encoder_arrow_timer == nullptr) {
            encoder_arrow_timer = lv_timer_create(encoderArrowTimer, 60, nullptr);
        }
    }
    // numeric readout removed (user requested)

    return screen;
}

void TensionMeasurementPage::firstRender() {
    // Turn off RGB LEDs initially for tension measurement page
    rgb.setColor(0, 0, 0);
}

void TensionMeasurementPage::lastRender() {
    // No special cleanup needed
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

    // Velocity ingestion with optional breakaway accumulation.
    // If BREAKAWAY_THRESHOLD is zero, skip accumulation and start moving immediately.
    if (BREAKAWAY_THRESHOLD > 0.0f && fabs(velocity) < VELOCITY_THRESHOLD) {
        // accumulate small movements until the threshold is reached
        breakaway_accumulator += movement;
        if (fabs(breakaway_accumulator) < BREAKAWAY_THRESHOLD) {
            // small visual ripple only
            wave_amplitude = fabs(breakaway_accumulator) * 0.25f;
            return;
        } else {
            // Apply accumulated force through smoothing and clear accumulator
            float applied = breakaway_accumulator;
            velocity = velocity * VELOCITY_SMOOTHING + applied * (1.0f - VELOCITY_SMOOTHING);
            breakaway_accumulator = 0.0f;
        }
    } else {
        // Normal path: apply movement into velocity using low-pass smoothing
        velocity = velocity * VELOCITY_SMOOTHING + movement * (1.0f - VELOCITY_SMOOTHING);
    }

    // Throttled debug (kept minimal)
    static unsigned long last_dbg = 0;
    if (millis() - last_dbg > 1000) {
        Serial.printf("TENSION d=%d mv=%.2f vel=%.2f pos=%.2f\n", delta, movement, velocity, current_position);
        last_dbg = millis();
    }

    // Create subtle wave ripple effect - keep it small so "weight" feels clearer
    wave_amplitude = fabs(velocity) * 0.35f;  // smaller visual ripple
    if (wave_amplitude > 6.0f) wave_amplitude = 6.0f;  // Cap at 6px

    // Update position immediately (current_position in 0..200 now)
    // Update position immediately (internal range 0..300)
    current_position += movement;

    // Update recent input activity (0..1). Larger movements increase activity which will
    // temporarily reduce braking and slightly increase coast in the momentum timer.
    float inst_activity = fabs(movement) / MAX_STEP_PER_TICK; // 0..1 (approx)
    if (inst_activity > 1.0f) inst_activity = 1.0f;
    // Smooth into activity state
    input_activity = input_activity * 0.72f + inst_activity * 0.28f;

    // Pulse directional arrow indicators for immediate feedback
    if (movement > 0.0f) {
        // Start an upward propagation from center outward
        encoder_arrow_up_activity = 1.0f;
        encoder_arrow_down_activity = 0.0f;
        encoder_arrow_progress_up = 0.0f;
        encoder_arrow_dir = 1;
        encoder_arrow_phase = 0.0f; // reset pulsing phase
        // Hold the tail expansion briefly so the ghost doesn't immediately contract
        encoder_arrow_hold_frames = ARROW_HOLD_FRAMES;
    } else if (movement < 0.0f) {
        // Start a downward propagation from center outward
        encoder_arrow_down_activity = 1.0f;
        encoder_arrow_up_activity = 0.0f;
        encoder_arrow_progress_down = 0.0f;
        encoder_arrow_dir = -1;
        encoder_arrow_phase = 0.0f;
        // Hold the tail expansion briefly so the ghost doesn't immediately contract
        encoder_arrow_hold_frames = ARROW_HOLD_FRAMES;
    }

    // Clamp to 0..300 range
    if (current_position < 0.0f) {
        current_position = 0.0f;
        velocity = 0.0f;  // Stop at boundary
    }
    if (current_position > 300.0f) {
        current_position = 300.0f;
        velocity = 0.0f;  // Stop at boundary
    }

    // Immediately reflect the manual adjustment without LVGL animation for responsiveness
    if (bar_obj) {
        int bar_value = (int)(current_position + 0.5f);
        // Reflect manual adjustment immediately for responsive feel
        lv_bar_set_value(bar_obj, bar_value, LV_ANIM_OFF);

        // Very slight RGB lighting: target color #42b2c2 (66,178,194)
        // Max brightness set to 5% (0.05). Scale linearly with current_position (0..300).
        float pos_frac = current_position / 300.0f; // 0..1
        float max_brightness = 0.05f; // 5%
        float brightness = pos_frac * max_brightness; // 0..0.05
        if (brightness <= 0.0005f) {
            // effectively off for very small values
            rgb.setColor(0, 0, 0);
        } else {
            uint8_t r = (uint8_t)roundf(66.0f * brightness);
            uint8_t g = (uint8_t)roundf(178.0f * brightness);
            uint8_t b = (uint8_t)roundf(194.0f * brightness);
            rgb.setColor(r, g, b);
        }

        // numeric readout removed (user requested)

        // Update scale label colors based on DISPLAYED value (0..10),
        // so label coloring stays consistent when internal scale changes.
        int disp_value = (int)(current_position / (300.0f / 10.0f) + 0.5f); // 0..10
        for (int i = 0; i < 6; i++) {
            int label_display = i * 2; // 0,2,4,6,8,10
            if (disp_value >= label_display) {
                lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0xFFFFFF), 0);
            } else {
                lv_obj_set_style_text_color(scale_labels[i], lv_color_hex(0x404040), 0);
            }
        }
    }
}

// Short encoder-driven animation was removed per user request to restore previous behavior

void TensionMeasurementPage::momentumTimerCallback(lv_timer_t *timer) {
    // Apply momentum decay
    if (fabs(velocity) > VELOCITY_THRESHOLD) {
        // Primary momentum decay; boost slightly when recent input activity is high so
        // fast flicks coast longer.
        float effective_decay = MOMENTUM_DECAY + (MOMENTUM_DECAY_BOOST * input_activity);
        if (effective_decay > 0.9999f) effective_decay = 0.9999f;
        velocity *= effective_decay;

        // Compute dynamic braking: reduce BRAKE_ALPHA when activity is high so fast
        // movements feel like they coast; slow movements still get stronger braking.
        float dynamic_brake = BRAKE_ALPHA * (1.0f - (BRAKE_DYNAMIC_REDUCTION * input_activity));
        if (dynamic_brake < 0.0f) dynamic_brake = 0.0f;

        // Apply additional braking (multiplicative)
        velocity *= (1.0f - dynamic_brake);

        // Advance position
        current_position += velocity;

        // Decay wave amplitude with velocity
        wave_amplitude *= MOMENTUM_DECAY;
        wave_phase += 0.3f;  // Advance wave animation

        // Snap-to-zero when velocity drops below configured stop threshold
        if (fabs(velocity) < STOP_VELOCITY) {
            velocity = 0.0f;
            wave_amplitude = 0.0f;
            // ensure display reflects final settled position
            if (current_position < 0.0f) current_position = 0.0f;
            if (current_position > 300.0f) current_position = 300.0f;
            updateDisplay();
            // decay the input activity on settle
            input_activity *= INPUT_ACTIVITY_DECAY;
            return;
        }

        // Clamp position during coast
        if (current_position < 0.0f) {
            current_position = 0.0f;
            velocity = 0.0f;
            wave_amplitude = 0.0f;
        }
        if (current_position > 300.0f) {
            current_position = 300.0f;
            velocity = 0.0f;
            wave_amplitude = 0.0f;
        }

        // decay recent input activity over time so braking returns to normal
        input_activity *= INPUT_ACTIVITY_DECAY;
        if (input_activity < 0.001f) input_activity = 0.0f;

        updateDisplay();
    } else {
        velocity = 0.0f;
        wave_amplitude = 0.0f;
    }
}

void TensionMeasurementPage::recordingLedTimer(lv_timer_t *timer) {
    if (!recording_led) return;
    // advance phase
    recording_phase += 0.18f;
    if (recording_phase > 10000.0f) recording_phase = fmodf(recording_phase, 6.2831853f);

    // Sine-based pulsing 0..1
    float v = (sinf(recording_phase) + 1.0f) * 0.5f; // 0..1
    // Map to LVGL opacity range (use 40..255)
    int opa = (int)(40 + v * (255 - 40));
    if (opa < 0) opa = 0;
    if (opa > 255) opa = 255;
    lv_obj_set_style_bg_opa(recording_led, (lv_opa_t)opa, 0);
}

void TensionMeasurementPage::encoderArrowTimer(lv_timer_t *timer) {
    // New behavior:
    // - Dragon heads are anchored to the current bar position (mapped to dot index)
    // - Both left/right dragons show the same head position
    // - Tail extends behind the head in the direction of motion (up -> tail below head, down -> tail above head)
    // - Tail length scales with velocity + recent input activity
    // - Head remains brightest and fades last
    const float DECAY = 0.78f;
    const int DOT_COUNT = TensionMeasurementPage::ARROW_DOT_COUNT;

    // advance pulsing phase for subtle shimmer
    encoder_arrow_phase += 0.28f;
    if (encoder_arrow_phase > 10000.0f) encoder_arrow_phase = fmodf(encoder_arrow_phase, 6.2831853f);
    float pulse = (sinf(encoder_arrow_phase * 2.0f) + 1.0f) * 0.5f; // 0..1

    // decay activities
    encoder_arrow_up_activity *= DECAY;
    encoder_arrow_down_activity *= DECAY;
    if (encoder_arrow_up_activity < 0.0005f) encoder_arrow_up_activity = 0.0f;
    if (encoder_arrow_down_activity < 0.0005f) encoder_arrow_down_activity = 0.0f;

    // Map current bar DISPLAYED value to head index (0 = top, DOT_COUNT-1 = bottom)
    // Using the displayed bar value keeps dots synced with LVGL animation.
    float pos_frac = 0.0f;
    if (bar_obj) {
        int disp = lv_bar_get_value(bar_obj);
        pos_frac = (float)disp / 300.0f;
    } else {
        pos_frac = current_position / 300.0f;
    }
    float head_f = (1.0f - pos_frac) * (float)(DOT_COUNT - 1);
    int head_idx = (int)roundf(head_f);
    if (head_idx < 0) head_idx = 0;
    if (head_idx > DOT_COUNT - 1) head_idx = DOT_COUNT - 1;

    // Decide motion direction: prefer instantaneous velocity, fall back to last encoder_arrow_dir or activity
    int motion_dir = 0;
    if (fabs(velocity) > VELOCITY_THRESHOLD) motion_dir = (velocity > 0.0f) ? 1 : -1;
    else if (encoder_arrow_dir != 0) motion_dir = encoder_arrow_dir;
    else if (encoder_arrow_up_activity > encoder_arrow_down_activity) motion_dir = 1;
    else if (encoder_arrow_down_activity > encoder_arrow_up_activity) motion_dir = -1;

    // Tail length (in dot counts) scales with velocity and activity.
    // Increase multipliers to produce longer motion-blur (ghost) tails.
    float activity = fmaxf(encoder_arrow_up_activity, encoder_arrow_down_activity);
    float vel_influence = fminf(fabs(velocity) * 0.10f, (float)(DOT_COUNT - 1));
    // stronger multipliers: activity contributes more, velocity contributes more
    int tail_len = 1 + (int)roundf(fminf(activity * 14.0f + vel_influence * 2.2f, (float)(DOT_COUNT - 1)));
    if (tail_len < 1) tail_len = 1;

    // Shrink the tail when the head approaches an edge so motion blur "zawija" smoothly.
    bool at_top = (head_idx == 0);
    bool at_bottom = (head_idx == (DOT_COUNT - 1));

    // Hold frames: when non-zero we delay any tail contraction so the ghost remains
    // briefly during motion transitions. Decrement each timer tick.
    bool hold_active = false;
    if (encoder_arrow_hold_frames > 0) {
        encoder_arrow_hold_frames -= 1;
        hold_active = true;
    }

    float edge_shrink = 1.0f;
    if (motion_dir == 1) {
        // moving up -> shrink proportionally to distance to top (head_idx 0 = at top)
        edge_shrink = (float)head_idx / (float)(DOT_COUNT - 1);
    } else if (motion_dir == -1) {
        // moving down -> shrink proportionally to distance to bottom
        edge_shrink = (float)(DOT_COUNT - 1 - head_idx) / (float)(DOT_COUNT - 1);
    } else {
        // idle: allow small shrink based on proximity to either edge
        float dtop = (float)head_idx / (float)(DOT_COUNT - 1);
        float dbot = (float)(DOT_COUNT - 1 - head_idx) / (float)(DOT_COUNT - 1);
        edge_shrink = fminf(dtop, dbot);
    }
    if (edge_shrink < 0.0f) edge_shrink = 0.0f;
    // apply shrink to tail length; if hold is active, skip shrinking so the ghost
    // remains for the hold duration
    if (!hold_active) {
        tail_len = (int)roundf((float)tail_len * edge_shrink);
    }
    if (at_top || at_bottom) tail_len = 0;

    // Smooth contraction: if tail length is decreasing, make it reduce more slowly
    // so the ghost contracts ~2x slower than the bar stopping. Expansion is immediate.
    int last_tail = encoder_arrow_last_tail_len;
    if (tail_len < last_tail) {
        int diff = last_tail - tail_len;
        int reduce = (diff + 1) / 2; // shrink by roughly half the difference per tick
        tail_len = last_tail - reduce;
        if (tail_len < 0) tail_len = 0;
    }
    encoder_arrow_last_tail_len = tail_len;

    // global brightness factor to keep dots subdued relative to the bar
    const float DOT_BRIGHTNESS = 0.55f; // ~55% of previous intensity

    // For each dot, compute intensity based on distance from head in motion direction
    for (int p = 0; p < DOT_COUNT; ++p) {
        // default off
        float intensity = 0.0f;
        // left/right parts use same logic
        if (motion_dir == 1) {
            // moving up -> head moves toward smaller index; tail is below head (indices > head_idx)
            int dist = p - head_idx; // 0 at head, positive when below head
            if (dist == 0) {
                // head: less intense than before to meet user's preference
                intensity = 0.48f + 0.12f * pulse + 0.15f * activity;
            } else if (dist > 0 && dist <= tail_len) {
                float fall = 1.0f - ((float)dist / (float)(tail_len + 1));
                intensity = (0.6f + 0.4f * pulse) * fall * (0.5f + 0.5f * activity);
            }
        } else if (motion_dir == -1) {
            // moving down -> head moves toward larger index; tail is above head (indices < head_idx)
            int dist = head_idx - p; // 0 at head, positive when above head
            if (dist == 0) {
                // head: less intense
                intensity = 0.48f + 0.12f * pulse + 0.15f * activity;
            } else if (dist > 0 && dist <= tail_len) {
                float fall = 1.0f - ((float)dist / (float)(tail_len + 1));
                intensity = (0.6f + 0.4f * pulse) * fall * (0.5f + 0.5f * activity);
            }
        } else {
            // idle: prefer a single head dot in standby. If there was very recent
            // activity we allow one faint neighbor; otherwise only the head is shown dimly.
            const float IDLE_ACTIVITY_THRESH = 0.02f;
            if (activity < IDLE_ACTIVITY_THRESH) {
                int dist = abs(p - head_idx);
                if (dist == 0) {
                    // subtle standby glow for the head
                    intensity = 0.28f + 0.12f * pulse;
                } else {
                    intensity = 0.0f;
                }
            } else {
                // recent small activity but not moving: show head and at most one faint neighbor
                int dist = abs(p - head_idx);
                if (dist == 0) intensity = 0.65f + 0.35f * pulse;
                else if (dist == 1) intensity = (0.25f + 0.20f * pulse) * activity;
                else intensity = 0.0f;
            }
        }

        // Apply global brightness then clamp and apply to both left and right dot objects
        intensity *= DOT_BRIGHTNESS;
        if (intensity < 0.0005f) intensity = 0.0f;
        if (intensity > 1.0f) intensity = 1.0f;

        lv_color_t col = lv_color_hex(0xFFFFFF);
        if (encoder_arrow_up_parts[p]) {
            lv_obj_set_style_bg_color(encoder_arrow_up_parts[p], col, 0);
            lv_obj_set_style_bg_opa(encoder_arrow_up_parts[p], (lv_opa_t)(int)(255.0f * intensity), 0);
        }
        if (encoder_arrow_down_parts[p]) {
            lv_obj_set_style_bg_color(encoder_arrow_down_parts[p], col, 0);
            lv_obj_set_style_bg_opa(encoder_arrow_down_parts[p], (lv_opa_t)(int)(255.0f * intensity), 0);
        }
    }

    // keep encoder_arrow_dir in sync with observed motion
    encoder_arrow_dir = motion_dir;
}

// encoder knob timer removed along with the knob/dots per user request

void TensionMeasurementPage::updateDisplay() {
    if (!bar_obj) return;

    int bar_value = (int)(current_position + 0.5f);  // Round to nearest int (0..300)
    // Use LVGL animation for momentum-driven updates to keep coasting smooth
    lv_bar_set_value(bar_obj, bar_value, LV_ANIM_ON);

    // Update RGB lighting subtly based on position -> target color #42b2c2
    float pos_frac = current_position / 300.0f; // 0..1
    float max_brightness = 0.05f; // 5%
    float brightness = pos_frac * max_brightness; // 0..0.05
    if (brightness <= 0.0005f) {
        rgb.setColor(0, 0, 0);
    } else {
        uint8_t r = (uint8_t)roundf(66.0f * brightness);
        uint8_t g = (uint8_t)roundf(178.0f * brightness);
        uint8_t b = (uint8_t)roundf(194.0f * brightness);
        rgb.setColor(r, g, b);
    }

    // Usunięcie łuku - bar ma zaokrągloną górną krawędź
    // Gradientowa kurtyna: blendowanie cyfr przez pionowy gradient opacity
    // (realizowane przez styl bara LVGL)

    // Blendowanie cyfr przez szarą kurtynę: colorujemy według WARSTWY WYŚWIETLANEJ (0..10),
    // aby synchronizacja etykiet nie zależała bezpośrednio od wewnętrznej skali paska.
    int disp_value = (int)(current_position / (300.0f / 10.0f) + 0.5f); // 0..10
    for (int i = 0; i < 6; i++) {
        int label_display = i * 2; // 0,2,4,6,8,10
        if (disp_value >= label_display) {
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
    if (current_position > 300.0f) current_position = 300.0f;
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

    if (recording_led_timer) {
        lv_timer_del(recording_led_timer);
        recording_led_timer = nullptr;
    }
    if (recording_led) {
        lv_obj_del(recording_led);
        recording_led = nullptr;
    }
    if (recording_label) {
        lv_obj_del(recording_label);
        recording_label = nullptr;
    }
    // Remove scalloped encoder tooth pieces if present
    for (int i = 0; i < 3; ++i) {
        if (encoder_tooth_parts[i]) {
            lv_obj_del(encoder_tooth_parts[i]);
            encoder_tooth_parts[i] = nullptr;
        }
    }
    if (encoder_tooth_reflection) {
        lv_obj_del(encoder_tooth_reflection);
        encoder_tooth_reflection = nullptr;
    }
    if (encoder_arrow_timer) {
        lv_timer_del(encoder_arrow_timer);
        encoder_arrow_timer = nullptr;
    }
    for (int p = 0; p < TensionMeasurementPage::ARROW_DOT_COUNT; ++p) {
        if (encoder_arrow_up_parts[p]) {
            lv_obj_del(encoder_arrow_up_parts[p]);
            encoder_arrow_up_parts[p] = nullptr;
        }
        if (encoder_arrow_down_parts[p]) {
            lv_obj_del(encoder_arrow_down_parts[p]);
            encoder_arrow_down_parts[p] = nullptr;
        }
    }
}
