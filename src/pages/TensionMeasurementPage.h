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
    static lv_obj_t *scale_labels[6];   // 6 etykiet pokazujących 0..10 (mapped from 0..300)
    static lv_style_t style_indic;
    static float current_position;  // 0.0-300.0 for smooth animation (visual range 0-300)
    static float velocity;          // Current velocity for momentum
    static float wave_amplitude;    // Wave ripple amplitude (decays with velocity)
    static float wave_phase;        // Wave animation phase
    static float breakaway_accumulator; // Accumulates small encoder movements until breakaway threshold
    static lv_timer_t *momentum_timer;
    static void momentumTimerCallback(lv_timer_t *timer);
    static void updateDisplay();
    static void drawWaveEffect();
    // Previously had short encoder-driven animation helpers; reverted per user request
    
    // Momentum physics parameters (tuned for smoother motion, larger 0..200 range)
    // Base sensitivity for raw diffs (lower to reduce jitter)
    // Further tuned (offline) for very smooth feel across 0..200 range
    static constexpr float ENCODER_SENSITIVITY = 2.00f;   // further increased for much fewer rotations
    // Adaptive sensitivity: scale movement when delta is large (less aggressive)
    static constexpr float ADAPTIVE_GAIN = 0.20f;        // stronger adaptive scaling
    static constexpr int ADAPTIVE_DELTA_MAX = 80;        // larger delta window for aggressive turns
    // Max movement allowed per tick to avoid jumps (balanced for 0..300)
    static constexpr float MAX_STEP_PER_TICK = 24.0f;    // allow much larger per-tick steps for responsiveness

    // Momentum: increase inertia for heavier movement (slower decay)
    // Increase inertia and smoothing so "weight" is visible
    static constexpr float MOMENTUM_DECAY = 0.994f;    // closer to 1 => longer coast
    static constexpr float VELOCITY_THRESHOLD = 0.06f; // ignore tiny jitter
    static constexpr uint32_t MOMENTUM_INTERVAL = 16;  // update rate (ms)

    // Velocity smoothing: stronger low-pass so motion feels heavier
    static constexpr float VELOCITY_SMOOTHING = 0.92f; // high smoothing => heavy feel

    // Breakaway / static friction: require accumulated input to exceed this to start movement
    static constexpr float BREAKAWAY_THRESHOLD = 6.0f; // small static friction to emphasize weight
    // no animation state
};
