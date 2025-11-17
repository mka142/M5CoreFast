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
#include "pages/SliderDemoPage.h"
#include "pages/TensionMeasurementPage.h"

// Widgets
#include <SponsorCarousel.h>

// Adapters
#include <HMIAdapter.h>
#include <RGBAdapter.h>

// Display resolution
constexpr int32_t HOR_RES = 320;
constexpr int32_t VER_RES = 240;

// Global objects
PageNavigator navigator;
HMIAdapter hmi;
RGBAdapter rgb;

// Current encoder value for UI updates
int lastEncoderValue = 0;
int tensionBufferCount = 0;

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
    hmi.begin();
    Serial.println("8. Adapters initialized!");
    
    // Create all screens
    Serial.println("9. Creating screens...");
    lv_obj_t *sponsorsScreen = SponsorsPage::create();
    lv_obj_t *loadingScreen = LoadingPage::create();
    lv_obj_t *beforeConcertScreen = BeforeConcertPage::create();
    lv_obj_t *sliderScreen = SliderDemoPage::create();
    lv_obj_t *tensionScreen = TensionMeasurementPage::create();
    Serial.println("10. All screens created!");
    
    // Register screens with navigator
    Serial.println("11. Registering screens...");
    navigator.registerScreen(SPONSORS, sponsorsScreen);
    navigator.registerScreen(LOADING, loadingScreen);
    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    navigator.registerScreen(SLIDER_DEMO, sliderScreen);
    navigator.registerScreen(TENSION_MEASUREMENT, tensionScreen);
    Serial.println("12. Screens registered!");
    
    // Show sponsors page initially
    Serial.println("13. Showing SPONSORS page...");
    navigator.showPage(SPONSORS);
    Serial.println("14. SPONSORS page displayed!");
    
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
    
    counter++;
    if (millis() - lastPrint > 5000) {
        Serial.printf("Loop running... counter=%d\n", counter);
        lastPrint = millis();
    }
    
    // Handle HMI input for page navigation
    static bool btnA_pressed = false;
    static bool btnB_pressed = false;
    
    bool btnA = hmi.getButtonA();
    bool btnB = hmi.getButtonB();
    
    // Button A: cycle through pages
    if (btnA && !btnA_pressed) {
        btnA_pressed = true;
        PageID current = navigator.getCurrentPage();
        
        switch(current) {
            case SPONSORS:
                navigator.showPage(LOADING);
                rgb.setColor(100, 100, 100);  // Gray
                Serial.println("-> LOADING");
                break;
            case LOADING:
                navigator.showPage(BEFORE_CONCERT);
                rgb.setColor(0, 0, 255);  // Blue
                Serial.println("-> BEFORE_CONCERT");
                break;
            case BEFORE_CONCERT:
                navigator.showPage(SLIDER_DEMO);
                rgb.setColor(0, 255, 0);  // Green
                Serial.println("-> SLIDER_DEMO");
                break;
            case SLIDER_DEMO:
                navigator.showPage(TENSION_MEASUREMENT);
                rgb.setColor(255, 255, 0);  // Yellow
                Serial.println("-> TENSION_MEASUREMENT");
                break;
            case TENSION_MEASUREMENT:
                navigator.showPage(SPONSORS);  // Wróć do sponsorów na końcu
                rgb.setColor(255, 255, 255);  // White
                Serial.println("-> SPONSORS (END)");
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
    
    delay(5);
}
