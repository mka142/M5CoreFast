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
    static float input_activity;    // 0..1 recent encoder activity (used to modulate braking/momentum)
    static lv_timer_t *momentum_timer;
    // Recording LED (bottom-left) and its timer
    static lv_obj_t *recording_led;
    static lv_timer_t *recording_led_timer;
    static float recording_phase;
    static void recordingLedTimer(lv_timer_t *timer);
    static lv_obj_t *recording_label;
    // Small scalloped encoder tooth pieces (protruding from left edge)
    static lv_obj_t *encoder_tooth_parts[3];
    static lv_obj_t *encoder_tooth_reflection;
    // Directional dot strips that flash when encoder is turned up/down
    // Use a dense set of dots per side for a smooth ghost animation
    static constexpr int ARROW_DOT_COUNT = 20; // adjust for density
    static lv_obj_t *encoder_arrow_up_parts[ARROW_DOT_COUNT];
    static lv_obj_t *encoder_arrow_down_parts[ARROW_DOT_COUNT];
    static lv_timer_t *encoder_arrow_timer;
    static float encoder_arrow_up_activity;
    static float encoder_arrow_down_activity;
    static float encoder_arrow_phase; // used to create pulsing during movement
    static float encoder_arrow_progress_up; // propagation progress for up-side dots (front position)
    static float encoder_arrow_progress_down; // propagation progress for down-side dots (front position)
    static float encoder_arrow_target_idx; // target index based on bar edge
    static int encoder_arrow_dir; // +1 up, -1 down, 0 idle
    // Hold frames to delay tail contraction when motion changes or starts
    static int encoder_arrow_hold_frames;
    static constexpr int ARROW_HOLD_FRAMES = 4; // number of timer ticks to hold (≈4*60ms ≈ 240ms)
    // Remember last tail length so contraction can be smoothed (slower than expansion)
    static int encoder_arrow_last_tail_len;
    static void encoderArrowTimer(lv_timer_t *timer);
    // Top-left illustrative knob removed; keep only recording LED + label on left
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
    static constexpr float MOMENTUM_DECAY = 0.996f;    // closer to 1 => longer coast
    // Extra decay gained when input activity is high (makes fast flicks coast longer)
    static constexpr float MOMENTUM_DECAY_BOOST = 0.0025f;
    static constexpr float VELOCITY_THRESHOLD = 0.06f; // ignore tiny jitter
    static constexpr uint32_t MOMENTUM_INTERVAL = 16;  // update rate (ms)

    // Velocity smoothing: stronger low-pass so motion feels heavier
    static constexpr float VELOCITY_SMOOTHING = 0.94f; // slightly less smoothing to keep responsiveness
    // How quickly the input_activity state decays per momentum tick (0..1)
    static constexpr float INPUT_ACTIVITY_DECAY = 0.88f;
    // How much braking is reduced at max activity (0..1). 1.0 -> full reduction, 0.0 -> no reduction
    static constexpr float BRAKE_DYNAMIC_REDUCTION = 0.85f;

    // Breakaway / static friction: require accumulated input to exceed this to start movement
    static constexpr float BREAKAWAY_THRESHOLD = 8.0f; // moderate static friction to emphasize weight
    // Braking parameters to provide smooth, finite stop
    static constexpr float BRAKE_ALPHA = 0.07f;       // gentler per-tick braking (less abrupt)
    static constexpr float STOP_VELOCITY = 0.35f;     // snap to zero below this velocity (snap later)
    // no animation state
};
