#pragma once
#include <lvgl.h>
#include <ThemeColors.h>

// Tension measurement page with vertical bar widget
class TensionMeasurementPage {
public:
    static lv_obj_t* create();
    static void updateValue(int value);
    static void updateBuffer(int count, int max);
    static void handleEncoder(int delta);
    static int getCurrentValue();
    static void cleanup();  // For momentum timer
    
private:
    static lv_obj_t *bar_obj;
    static lv_obj_t *center_line;       // Kreska środkowa
    static lv_obj_t *side_lines[3];     // 0: arrow_down, 1: arrow_left, 2: arrow_right
    static lv_obj_t *buffer_label;      // Keep buffer display
    // numeric readout removed per user request
    static lv_obj_t *scale_labels[6];   // 6 cyfr (0,2,4,6,8,10) na środku
    static lv_style_t style_indic;
    static float current_position;  // 0.0-100.0 for smooth animation (visual range 0-100)
    static float velocity;          // Current velocity for momentum
    static float wave_amplitude;    // Wave ripple amplitude (decays with velocity)
    static float wave_phase;        // Wave animation phase
    static lv_timer_t *momentum_timer;
    static void momentumTimerCallback(lv_timer_t *timer);
    static void updateDisplay();
    static void drawWaveEffect();
    
    // Momentum physics parameters
    // Use a lower sensitivity because we now pass raw encoder diffs
    static constexpr float ENCODER_SENSITIVITY = 0.75f;   // Base sensitivity for raw diffs
    // Adaptive sensitivity: scale movement when delta is large
    static constexpr float ADAPTIVE_GAIN = 0.18f;        // Additional multiplier per notch (clamped)
    static constexpr int ADAPTIVE_DELTA_MAX = 12;        // Max delta considered for adaptive gain
    static constexpr float MAX_STEP_PER_TICK = 8.0f;     // Max movement allowed per tick to avoid jumps

    // Tuning: stronger braking (lower decay) and slightly higher stop threshold
    static constexpr float MOMENTUM_DECAY = 0.86f;       // 0.75-0.92, lower = faster braking
    static constexpr float VELOCITY_THRESHOLD = 0.18f;   // 0.05-0.5, threshold to consider momentum
    static constexpr uint32_t MOMENTUM_INTERVAL = 30;    // 20-50ms, update rate
};
