#include <Arduino.h>
#include <M5CoreS3.h>

// LVGL - LV_CONF_INCLUDE_SIMPLE is defined in platformio.ini
#include <lvgl.h>

// Our simple display driver
#include <LVGLDisplay.h>

// Page management
#include <PageNavigator.h>
#include <PageID.h>

// Pages
#include "pages/SponsorsPage.h"
#include "pages/LoadingPage.h"
#include "pages/BeforeConcertPage.h"
#include "pages/AppGuidePage.h"
#include "pages/SliderDemoPage.h"
#include "pages/TensionMeasurementPage.h"
#include "pages/ResearchForm/ResearchFormPage.h"
#include "pages/ChargingPage.h"
#include "pages/OvationPage.h"

// Widgets
#include <SponsorCarousel.h>

// Adapters
#include <HMIAdapter.h>
#include <RGBAdapter.h>

// Display resolution
constexpr int32_t HOR_RES = 320;
constexpr int32_t VER_RES = 240;

// Feature flags
constexpr bool SHOW_CHARGING = true;  // Set to false to disable charging screen

// Global objects
PageNavigator navigator;
HMIAdapter hmi;
RGBAdapter rgb;

// Current encoder value for UI updates
int lastEncoderValue = 0;
int tensionBufferCount = 0;

// Charging state tracking
bool was_charging = false;
PageID page_before_charging = LOADING;
unsigned long last_battery_update = 0;

// Touch input for LVGL
void touchpad_read(lv_indev_t *drv, lv_indev_data_t *data) {
    M5.update();
    auto count = M5.Touch.getCount();
    
    if (count == 0) {
        data->state = LV_INDEV_STATE_RELEASED;
    } else {
        auto touch = M5.Touch.getDetail(0);
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch.x;
        data->point.y = touch.y;
    }
}

void setup() {
    // Initialize Serial FIRST with delays
    Serial.begin(115200);
    delay(2000);  // Longer delay for serial
    
    Serial.println("\n\n\n");
    Serial.println("================================");
    Serial.println("=== M5CoreFast LVGL ===");
    Serial.println("================================");
    
    // Initialize M5CoreS3
    Serial.println("1. Initializing M5CoreS3...");
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    Serial.println("2. M5CoreS3 initialized!");
    
    // Initialize LVGL
    Serial.println("3. Initializing LVGL...");
    LVGLDisplay::init(HOR_RES, VER_RES);
    Serial.println("4. LVGL initialized!");
    
    // Setup touch input
    Serial.println("5. Setting up touch input...");
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
    Serial.println("6. Touch input configured!");
    
    // Initialize hardware adapters
    Serial.println("7. Initializing adapters...");
    rgb.begin(5, 10);  // Pin 5, 10 LEDs
    rgb.setColor(0, 0, 0);  // Start with RGB off
    hmi.begin();
    Serial.println("8. Adapters initialized!");

    // ==================== SETUP SCREENS ====================
    // Create all screens
    Serial.println("9. Creating screens...");
    lv_obj_t *sponsorsScreen = SponsorsPage::create();
    lv_obj_t *loadingScreen = LoadingPage::create();
    lv_obj_t *beforeConcertScreen = BeforeConcertPage::create();
    lv_obj_t *appGuideScreen = AppGuidePage::create();
    lv_obj_t *sliderScreen = SliderDemoPage::create();
    lv_obj_t *tensionScreen = TensionMeasurementPage::create();
    lv_obj_t *researchFormScreen = ResearchFormPage::create();
    lv_obj_t *chargingScreen = ChargingPage::create();
    lv_obj_t *ovationScreen = OvationPage::create();
    Serial.println("10. All screens created!");
    
    // Register screens with navigator
    Serial.println("11. Registering screens...");
    navigator.registerScreen(SPONSORS, sponsorsScreen);
    navigator.registerScreen(LOADING, loadingScreen);
    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    navigator.registerScreen(APP_GUIDE, appGuideScreen);
    navigator.registerScreen(SLIDER_DEMO, sliderScreen);
    navigator.registerScreen(TENSION_MEASUREMENT, tensionScreen);
    navigator.registerScreen(BEFORE_CONCERT__RESEARCH_FORM, researchFormScreen);
    navigator.registerScreen(CHARGING, chargingScreen);
    navigator.registerScreen(OVATION, ovationScreen);
    Serial.println("12. Screens registered!");
    navigator.registerScreen(SPONSORS, sponsorsScreen);
    navigator.registerScreen(LOADING, loadingScreen);

    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    navigator.registerScreen(BEFORE_CONCERT__RESEARCH_FORM, researchFormScreen);

    navigator.registerScreen(SLIDER_DEMO, sliderScreen);
    navigator.registerScreen(TENSION_MEASUREMENT, tensionScreen);

    // Internal charging screen
    navigator.registerScreen(CHARGING, chargingScreen);
    Serial.println("12. Screens registered!");
    

    // ==================== CHARGING LOGIC SETUP ====================
    // Check if device is charging at startup (only if SHOW_CHARGING is enabled)
    bool is_charging = SHOW_CHARGING && M5.Power.isCharging();
    if (is_charging) {
        Serial.println("13. Device is charging - showing CHARGING page...");
        navigator.showPage(CHARGING);
        // Turn off RGB
        rgb.setColor(0, 0, 0);
        was_charging = true;
        Serial.println("14. CHARGING page displayed!");
    } else {
        // Show sponsors page initially
        Serial.println("13. Showing SPONSORS page...");
        navigator.showPage(SPONSORS);
        Serial.println("14. SPONSORS page displayed!");
    }
    
    Serial.println("================================");
    Serial.println("=== Setup Complete ===");
    Serial.println("Press Button A to cycle pages");
    Serial.println("================================");
}

void loop() {
    static unsigned long lastPrint = 0;
    static int counter = 0;
    
    M5.update();
    lv_task_handler();  // Handle LVGL tasks
    

    // ==================== DEBUG MEMORY LOGGING ====================
    counter++;
    if (millis() - lastPrint > 5000) {
        // Get memory information
        size_t free_heap = ESP.getFreeHeap();
        size_t total_heap = ESP.getHeapSize();
        size_t used_heap = total_heap - free_heap;
        size_t free_psram = ESP.getFreePsram();
        size_t total_psram = ESP.getPsramSize();
        size_t used_psram = total_psram - free_psram;
        
        Serial.printf("Loop running... counter=%d\n", counter);
        Serial.printf("Memory - Heap: %d/%d KB (%.1f%% used), PSRAM: %d/%d KB (%.1f%% used)\n",
                     used_heap / 1024, total_heap / 1024, (used_heap * 100.0f) / total_heap,
                     used_psram / 1024, total_psram / 1024, (used_psram * 100.0f) / total_psram);
        lastPrint = millis();
    }
    
    // ==================== CHARGING LOGIC LOOP ====================
    if (SHOW_CHARGING) {
        bool is_charging = M5.Power.isCharging();
        
        // If charging state changed to charging, switch to charging page
        if (is_charging && !was_charging) {
            Serial.println("Device started charging - showing CHARGING page");
            page_before_charging = navigator.getCurrentPage();
            navigator.showPage(CHARGING);
            rgb.setColor(0, 0, 0);  // Turn off RGB
            was_charging = true;
        }
        // If unplugged from charging, return to previous page
        else if (!is_charging && was_charging) {
            Serial.println("Device unplugged - returning to previous page");
            navigator.showPage(page_before_charging);
            was_charging = false;
        }
        
        // Update battery display every 2 seconds when on charging page
        if (navigator.getCurrentPage() == CHARGING) {
            if (millis() - last_battery_update > 2000) {
                ChargingPage::update_battery_status();
                last_battery_update = millis();
            }
            

            // ======== JUST FOR TESTING PURPOSES - DISMISS CHARGING WITH BUTTON A ========
            // // While on charging screen, ignore all other input except dismissal
            // // Check for touch/button to dismiss charging screen
            // static bool btnA_pressed_charging = false;
            // bool btnA = hmi.getButtonA();
            
            // if (btnA && !btnA_pressed_charging) {
            //     btnA_pressed_charging = true;
            //     Serial.println("Charging screen dismissed - returning to previous page");
            //     navigator.showPage(page_before_charging);
            //     was_charging = false;  // Prevent auto-return to charging
            // }
            // if (!btnA) btnA_pressed_charging = false;
            // ======== END TESTING CODE ===================================================
            
            // Early return - skip rest of loop while on charging screen
            delay(5);
            return;
        }
    }
    
    // ==================== NORMAL OPERATION ====================
    // Handle HMI input for page navigation
    static bool btnA_pressed = false;
    static bool btnB_pressed = false;
    
    bool btnA = hmi.getButtonA();
    bool btnB = hmi.getButtonB();
    
    // Button A: cycle through pages
    if (btnA && !btnA_pressed) {
        btnA_pressed = true;
        PageID current = navigator.getCurrentPage();
            // Normal page cycling
            // set rgb off
            rgb.setColor(0, 0, 0);
            switch(current) {
                 case LOADING:
                    navigator.showPage(APP_GUIDE);
                    rgb.setColor(50, 50, 150);  // Blue tint
                    Serial.println("-> APP_GUIDE");
                    break;
                case APP_GUIDE:
                    navigator.showPage(BEFORE_CONCERT);
                    // Set RGB sides: purple (0x9261D5) on left, cyan (0x42B2C2) on right
                    // Assuming 10 LEDs, split them: 0-4 cyan, 5-9 purple
                    for (int i = 0; i < 5; i++) {
                        rgb.setPixel(i, 0x42, 0xB2, 0xC2);  // Cyan
                    }
                    for (int i = 5; i < 10; i++) {
                        rgb.setPixel(i, 0x92, 0x61, 0xD5);  // Purple
                    }
                    rgb.show();
                    Serial.println("-> BEFORE_CONCERT");
                    break;
                case SPONSORS:
                    navigator.showPage(LOADING);
                    rgb.setColor(100, 100, 100);  // Gray
                    Serial.println("-> LOADING");
                    break;
               
                case BEFORE_CONCERT:
                    // Don't auto-navigate - user clicks button to go to RESEARCH_FORM
                    navigator.showPage(SLIDER_DEMO);
                    Serial.println("-> SLIDER_DEMO (for testing)");
                    break;
                case SLIDER_DEMO:
                    navigator.showPage(TENSION_MEASUREMENT);
                    Serial.println("-> TENSION_MEASUREMENT");
                    break;
                case TENSION_MEASUREMENT:
                    navigator.showPage(OVATION);
                    rgb.setColor(0, 0, 0);  // Off during ovation
                    Serial.println("-> OVATION");
                    break;
                case OVATION:
                    navigator.showPage(SPONSORS);
                    Serial.println("-> SPONSORS");
                    break;
                case BEFORE_CONCERT__RESEARCH_FORM:
                    // Research form navigates back to BEFORE_CONCERT via its submit button
                    navigator.showPage(BEFORE_CONCERT);
                    Serial.println("-> BEFORE_CONCERT (from form)");
                    break;
                default:
                    navigator.showPage(SPONSORS);
                    break;
            }
    }
    if (!btnA) btnA_pressed = false;
    
    // Button B: go back to sponsors
    if (btnB && !btnB_pressed) {
        btnB_pressed = true;
        navigator.showPage(SPONSORS);
        rgb.setColor(255, 255, 255);
        Serial.println("-> SPONSORS (back)");
    }
    if (!btnB) btnB_pressed = false;
    
    // Update tension page with encoder value
    if (navigator.getCurrentPage() == TENSION_MEASUREMENT) {
        int encoderValue = hmi.getEncoderValue();
        if (encoderValue != lastEncoderValue) {
            lastEncoderValue = encoderValue;
            TensionMeasurementPage::updateValue(encoderValue);
            
            // Simulate buffer filling
            tensionBufferCount++;
            if (tensionBufferCount > 200) tensionBufferCount = 0;
            TensionMeasurementPage::updateBuffer(tensionBufferCount, 200);
        }
    }


    // Small delay to avoid busy looping
    delay(5);
}
