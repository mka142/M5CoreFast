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
#include "pages/PieceAnnouncementPage.h"
#include "pages/EndOfConcertPage.h"
#include "pages/FeedbackForm/FeedbackFormPage.h"
#include "pages/FormFinishedPage.h"

// Widgets
#include <SponsorCarousel.h>

// Adapters
#include <HMIAdapter.h>
#include <RGBAdapter.h>
#include <WiFiAdapter.h>
#include <RTCAdapter.h>
#include <HTTPAdapter.h>
#include <MQTTAdapter.h>
#include <MQTTPageBridge.h>

// ==================== NETWORK CONFIGURATION ====================
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define NTP_TIMEZONE "UTC+2"
#define NTP_SERVER1 "0.pool.ntp.org"
#define NTP_SERVER2 "1.pool.ntp.org"
#define NTP_SERVER3 "2.pool.ntp.org"

// MQTT Configuration
#define MQTT_SERVER ""
#define MQTT_PORT 1883

// DEVICE_ID is set at build time via deploy script
// If not set, use a default value
#ifndef DEVICE_ID
#define DEVICE_ID "device_default"
#endif

// Convert macro to string for use in constructors
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
const char* MQTT_CLIENT_ID = TOSTRING(DEVICE_ID);

#define MQTT_USERNAME "your_username" // Optional
#define MQTT_PASSWORD "your_password" // Optional
#define MQTT_TOPIC_EVENTS "events/broadcast"
#define MQTT_TOPIC_STATUS "display/status"

// API Configuration
#define FORM_API_HOST ""


// Display resolution
constexpr int32_t HOR_RES = 320;
constexpr int32_t VER_RES = 240;

// Feature flags
constexpr bool SHOW_CHARGING = false;  // Set to false to disable charging screen

// Global objects
PageNavigator navigator;
HMIAdapter hmi;
RGBAdapter rgb;

// Network adapters
WiFiAdapter wifi;
RTCAdapter rtc;
HTTPAdapter httpAdapter;
MQTTAdapter* mqttAdapter = nullptr;
MQTTPageBridge* mqttPageBridge = nullptr;

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
    
    // Initialize LVGL EARLY so we can show status on screen
    Serial.println("3. Initializing LVGL...");
    LVGLDisplay::init(HOR_RES, VER_RES);
    Serial.println("4. LVGL initialized!");
    
    // Setup touch input
    Serial.println("5. Setting up touch input...");
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
    Serial.println("6. Touch input configured!");
    
    // Create a status screen to show initialization progress
    lv_obj_t* status_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(status_screen, lv_color_hex(0x000000), 0);
    lv_screen_load(status_screen);
    
    lv_obj_t* status_label = lv_label_create(status_screen);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_width(status_label, 280);
    lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(status_label, "Inicjalizacja...\n\nŁączenie z WiFi...");
    lv_task_handler();  // Force screen update
    
    // Initialize hardware adapters
    Serial.println("7. Initializing adapters...");
    rgb.begin(5, 10);  // Pin 5, 10 LEDs
    rgb.setColor(0, 0, 0);  // Start with RGB off
    hmi.begin();
    Serial.println("8. Adapters initialized!");
    
    // Initialize WiFi and RTC
    Serial.println("9. Initializing WiFi...");
    Serial.print("   Device ID: ");
    Serial.println(MQTT_CLIENT_ID);
    Serial.print("   SSID: ");
    Serial.println(WIFI_SSID);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Try to connect with timeout (10 seconds max)
    unsigned long wifi_start = millis();
    const unsigned long wifi_timeout = 10000;  // 10 seconds
    int dots = 0;
    while (!wifi.isConnected() && (millis() - wifi_start < wifi_timeout)) {
        unsigned long elapsed = (millis() - wifi_start) / 1000;
        Serial.print("   Connecting to WiFi");
        Serial.print("...");
        Serial.print(elapsed);
        Serial.println("s");
        
        // Update screen status
        char status_text[128];
        snprintf(status_text, sizeof(status_text), 
                "Inicjalizacja...\n\nŁączenie z WiFi:\n%s\n%lus / 10s", 
                WIFI_SSID, elapsed);
        lv_label_set_text(status_label, status_text);
        lv_task_handler();  // Force screen update
        
        wifi.loop();
        delay(500);
        dots++;
    }
    
    if (wifi.isConnected()) {
        Serial.println("10. WiFi connected!");
        lv_label_set_text(status_label, "Inicjalizacja...\n\nWiFi: Połączono!\n\nŁadowanie...");
        lv_task_handler();
        rgb.setColor(0, 50, 0);  // Green for WiFi success
        delay(1000);
    } else {
        Serial.println("10. WiFi connection timeout - continuing without WiFi");
        lv_label_set_text(status_label, "Inicjalizacja...\n\nWiFi: Brak połączenia\n(kontynuacja bez WiFi)\n\nŁadowanie...");
        lv_task_handler();
        rgb.setColor(50, 0, 0);  // Red for WiFi failure
        delay(2000);
    }
    
    Serial.println("11. Initializing RTC...");
    rtc.begin();
    // rtc.setLocalTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
    Serial.println("12. RTC initialized!");

    // ==================== SETUP SCREENS ====================
    // Create all screens
    Serial.println("13. Creating screens...");
    lv_label_set_text(status_label, "Tworzenie ekranów...");
    lv_task_handler();
    rgb.setColor(0, 0, 0);  // Reset RGB
    lv_obj_t *sponsorsScreen = SponsorsPage::create();
    lv_obj_t *loadingScreen = LoadingPage::create();
    lv_obj_t *beforeConcertScreen = BeforeConcertPage::create();
    lv_obj_t *appGuideScreen = AppGuidePage::create();
    lv_obj_t *sliderScreen = SliderDemoPage::create();
    lv_obj_t *tensionScreen = TensionMeasurementPage::create();
    lv_obj_t *researchFormScreen = ResearchFormPage::create();
    lv_obj_t *chargingScreen = ChargingPage::create();
    lv_obj_t *ovationScreen = OvationPage::create();
    lv_obj_t *pieceAnnouncementScreen = PieceAnnouncementPage::create();
    lv_obj_t *endOfConcertScreen = EndOfConcertPage::create();
    lv_obj_t *feedbackFormScreen = FeedbackFormPage::create();
    lv_obj_t *formFinishedScreen = FormFinishedPage::create();
    Serial.println("14. All screens created!");
    
    // Register screens with navigator
    Serial.println("15. Registering screens...");
    navigator.registerScreen(SPONSORS, sponsorsScreen);
    navigator.registerScreen(LOADING, loadingScreen);
    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    navigator.registerScreen(APP_GUIDE, appGuideScreen);
    navigator.registerScreen(SLIDER_DEMO, sliderScreen);
    navigator.registerScreen(TENSION_MEASUREMENT, tensionScreen);
    navigator.registerScreen(BEFORE_CONCERT__RESEARCH_FORM, researchFormScreen);
    navigator.registerScreen(CHARGING, chargingScreen);
    navigator.registerScreen(OVATION, ovationScreen);
    navigator.registerScreen(PIECE_ANNOUNCEMENT, pieceAnnouncementScreen);
    navigator.registerScreen(END_OF_CONCERT, endOfConcertScreen);
    navigator.registerScreen(END_OF_CONCERT__FEEDBACK_FORM, feedbackFormScreen);
    navigator.registerScreen(END_OF_CONCERT__FORM_FINISHED, formFinishedScreen);
    Serial.println("16. Screens registered!");
    
    // Set mock data for piece announcement page
    PieceAnnouncementPage::setComposer("Fryderyk Chopin");
    PieceAnnouncementPage::setPiece("Nokturno cis-moll op. 27 nr 1");
    PieceAnnouncementPage::setPerformers("Jan Kowalski - fortepian\nAnna Nowak - skrzypce");
    PieceAnnouncementPage::setDescription("Nokturno powstało w 1836 roku i jest jednym z najpiękniejszych utworów Chopina. Charakteryzuje się melancholijnym nastrojem i bogatą harmonią.");

    // ==================== INITIALIZE HTTP AND MQTT ====================
    if (wifi.isConnected()) {
        Serial.println("17. Initializing HTTP adapter...");
        lv_label_set_text(status_label, "Inicjalizacja HTTP...");
        lv_task_handler();
        httpAdapter.begin();
        Serial.println("18. HTTP adapter initialized!");
        
        Serial.println("19. Creating MQTT adapter...");
        mqttAdapter = new MQTTAdapter(MQTT_SERVER, MQTT_PORT, MQTT_CLIENT_ID);
        mqttPageBridge = new MQTTPageBridge(*mqttAdapter, navigator);
        Serial.println("20. MQTT objects created!");
        
        Serial.println("21. Initializing MQTT adapter...");
        lv_label_set_text(status_label, "Inicjalizacja MQTT...");
        lv_task_handler();
        mqttAdapter->begin(MQTT_USERNAME, MQTT_PASSWORD);
        mqttAdapter->subscribeTo(MQTT_TOPIC_EVENTS);
        Serial.println("22. MQTT adapter initialized!");
        
        Serial.println("23. Setting up MQTT page bridge...");
        lv_label_set_text(status_label, "Konfiguracja MQTT...");
        lv_task_handler();
        mqttPageBridge->begin();
        mqttPageBridge->addPageMapping("SPONSORS", SPONSORS);
        mqttPageBridge->addPageMapping("LOADING", LOADING);
        mqttPageBridge->addPageMapping("BEFORE_CONCERT", BEFORE_CONCERT);
        mqttPageBridge->addPageMapping("BEFORE_CONCERT__RESEARCH_FORM", BEFORE_CONCERT__RESEARCH_FORM);
        mqttPageBridge->addPageMapping("APP_GUIDE", APP_GUIDE);
        mqttPageBridge->addPageMapping("SLIDER_DEMO", SLIDER_DEMO);
        mqttPageBridge->addPageMapping("TENSION_MEASUREMENT", TENSION_MEASUREMENT);
        mqttPageBridge->addPageMapping("OVATION", OVATION);
        mqttPageBridge->addPageMapping("PIECE_ANNOUNCEMENT", PIECE_ANNOUNCEMENT);
        mqttPageBridge->addPageMapping("END_OF_CONCERT", END_OF_CONCERT);
        mqttPageBridge->addPageMapping("END_OF_CONCERT__FEEDBACK_FORM", END_OF_CONCERT__FEEDBACK_FORM);
        mqttPageBridge->addPageMapping("END_OF_CONCERT__FORM_FINISHED", END_OF_CONCERT__FORM_FINISHED);
        
        // Register payload handlers for pages that need event data
        mqttPageBridge->registerPayloadHandler(OVATION, OvationPage::setPayload);
        mqttPageBridge->registerPayloadHandler(PIECE_ANNOUNCEMENT, PieceAnnouncementPage::setPayload);
        
        Serial.println("24. MQTT page bridge configured!");
    } else {
        Serial.println("17. Skipping HTTP/MQTT initialization - no WiFi connection");
        lv_label_set_text(status_label, "Pomijanie HTTP/MQTT\n(brak WiFi)");
        lv_task_handler();
        delay(1000);
    }

    // Show "Ready!" message before switching to main screen
    Serial.println("25. Initialization complete!");
    lv_label_set_text(status_label, "Gotowe!\n\nPrzełączanie...");
    lv_task_handler();
    delay(500);
    
    // Clean up status screen (will be replaced by first page)
    lv_obj_del(status_screen);

    // ==================== CHARGING LOGIC SETUP ====================
    // Check if device is charging at startup (only if SHOW_CHARGING is enabled)
    bool is_charging = SHOW_CHARGING && M5.Power.isCharging();
    if (is_charging) {
        Serial.println("26. Device is charging - showing CHARGING page...");
        navigator.showPage(CHARGING);
        // Turn off RGB
        rgb.setColor(0, 0, 0);
        was_charging = true;
        Serial.println("27. CHARGING page displayed!");
    } else {
        // Show sponsors page initially
        Serial.println("26. Showing SPONSORS page...");
        navigator.showPage(SPONSORS);
        Serial.println("27. SPONSORS page displayed!");
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
    
    // ==================== NETWORK ADAPTERS ====================
    // Always call wifi.loop() to maintain connection
    wifi.loop();
    
    // Only call MQTT/HTTP if we have WiFi and adapters are initialized
    if (wifi.isConnected() && mqttAdapter != nullptr && mqttPageBridge != nullptr) {
        mqttAdapter->loop();
        mqttPageBridge->loop();
        httpAdapter.loop();
    }

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
                    navigator.showPage(PIECE_ANNOUNCEMENT);
                    rgb.setColor(0, 0, 0);  // Off during announcement
                    Serial.println("-> PIECE_ANNOUNCEMENT");
                    break;
                case PIECE_ANNOUNCEMENT:
                    navigator.showPage(TENSION_MEASUREMENT);
                    Serial.println("-> TENSION_MEASUREMENT");
                    break;
                case TENSION_MEASUREMENT:
                    navigator.showPage(OVATION);
                    rgb.setColor(0, 0, 0);  // Off during ovation
                    Serial.println("-> OVATION");
                    break;
                case OVATION:
                    navigator.showPage(END_OF_CONCERT);
                    Serial.println("-> END_OF_CONCERT");
                    break;
                case END_OF_CONCERT:
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
    
    // Update tension page with encoder value (with momentum physics)
    if (navigator.getCurrentPage() == TENSION_MEASUREMENT) {
        int encoderValue = hmi.getEncoderValue();
        int rawDiff = encoderValue - lastEncoderValue;
        int delta = rawDiff;  // use raw diff for proportional response

        // Throttled debug print to observe encoder behavior
        static unsigned long lastEncLog = 0;
        if (millis() - lastEncLog > 200) {
            Serial.printf("ENCODER raw=%d last=%d diff=%d delta=%d\n", encoderValue, lastEncoderValue, rawDiff, delta);
            lastEncLog = millis();
        }

        if (delta != 0) {
            lastEncoderValue += delta;
            TensionMeasurementPage::handleEncoder(delta);

            // Simulate buffer filling (kept for compatibility; label is commented out)
            tensionBufferCount++;
            if (tensionBufferCount > 200) tensionBufferCount = 0;
            TensionMeasurementPage::updateBuffer(tensionBufferCount, 200);
        }
    }


    // Small delay to avoid busy looping
    delay(5);
}
