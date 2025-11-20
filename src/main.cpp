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
#include <EventPoller.h>       // HTTP polling for events
#include <MQTTAdapterESP.h>    // Using ESP-IDF MQTT with WebSocket support (currently disabled)
#include <MQTTPageBridge.h>

// ==================== NETWORK CONFIGURATION ====================
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define NTP_TIMEZONE "UTC+2"
#define NTP_SERVER1 "0.pool.ntp.org"
#define NTP_SERVER2 "1.pool.ntp.org"
#define NTP_SERVER3 "2.pool.ntp.org"

// MQTT Configuration (currently disabled - using HTTP polling instead)
#define MQTT_SERVER "server.device-manager.fast.knakitm.pl"
#define MQTT_PORT 443        // Port 443 for WSS (WebSocket Secure)
#define MQTT_USE_WSS false   // Disabled: TLS verification issues
#define MQTT_WS_PATH "/mqtt" // WebSocket path on server
#define USE_MQTT false       // Set to true to use MQTT, false to use HTTP polling

// HTTP Polling Configuration
#define EVENT_API_ENDPOINT "https://server.device-manager.fast.knakitm.pl/api/concert/currentEvent"
#define POLL_INTERVAL_MS 5000  // Poll every 5 seconds

// USER_ID is set at build time via deploy script
// If not set, use a default value
#ifndef USER_ID
#define USER_ID user_default
#endif

// Convert macro to string - USER_ID comes without quotes from build system
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
const char *MQTT_CLIENT_ID = TOSTRING(USER_ID);
const char *USER_ID_STR = TOSTRING(USER_ID);

#define MQTT_USERNAME "your_username" // Optional
#define MQTT_PASSWORD "your_password" // Optional
#define MQTT_TOPIC_EVENTS "events/broadcast"
#define MQTT_TOPIC_STATUS "display/status"

// Examination Form API Configuration
const char *FORM_ID_RESEARCH = "concert-preexamination-form";
const char *FORM_ID_FEEDBACK = "concert-feedback-form";
const char *EXAM_FORM_SUBMIT_ENDPOINT = "https://server.device-manager.fast.knakitm.pl/api/examination-forms";
const char *EXAM_FORM_GET_RESPONSE_ENDPOINT = "https://server.device-manager.fast.knakitm.pl/api/examination-forms/user";
const char *FORM_BATCH_API_ENDPOINT = "https://server.device-manager.fast.knakitm.pl/api/forms/batch";

// Display resolution
constexpr int32_t HOR_RES = 320;
constexpr int32_t VER_RES = 240;

// Feature flags
constexpr bool SHOW_CHARGING = false; // Set to false to disable charging screen
constexpr bool PREVIEW_MODE = false; // Set to true to skip WiFi/MQTT for and testing UI only

// Global objects
PageNavigator navigator;
HMIAdapter hmi;
RGBAdapter rgb;

// Network adapters
WiFiAdapter wifi;
RTCAdapter rtc;
HTTPAdapter httpAdapter;
EventPoller *eventPoller = nullptr;        // HTTP polling for events
MQTTAdapterESP *mqttAdapter = nullptr;     // MQTT (currently disabled)
MQTTPageBridge *mqttPageBridge = nullptr;  // MQTT bridge (currently disabled)

// Current encoder value for UI updates
int lastEncoderValue = 0;
int tensionBufferCount = 0;

// Charging state tracking
bool was_charging = false;
PageID page_before_charging = LOADING;
unsigned long last_battery_update = 0;

// Touch input for LVGL
void touchpad_read(lv_indev_t *drv, lv_indev_data_t *data)
{
    M5.update();
    auto count = M5.Touch.getCount();

    if (count == 0)
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    else
    {
        auto touch = M5.Touch.getDetail(0);
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch.x;
        data->point.y = touch.y;
    }
}

void setup()
{
    // Initialize Serial FIRST with delays
    Serial.begin(115200);
    delay(2000); // Longer delay for serial

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
    lv_obj_t *status_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(status_screen, lv_color_hex(0x000000), 0);
    lv_screen_load(status_screen);

    lv_obj_t *status_label = lv_label_create(status_screen);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_width(status_label, 280);
    lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(status_label, "Inicjalizacja...\n\nŁączenie z WiFi...");
    lv_task_handler(); // Force screen update

    // Initialize hardware adapters
    Serial.println("7. Initializing adapters...");
    rgb.begin(5, 10);      // Pin 5, 10 LEDs
    rgb.setColor(0, 0, 0); // Start with RGB off
    hmi.begin();
    Serial.println("8. Adapters initialized!");

    // ==================== CHECK CHARGING MODE ====================
    // If device is charging and SHOW_CHARGING is enabled, skip all network setup
    bool is_charging = SHOW_CHARGING && M5.Power.isCharging();
    bool skip_network = is_charging || PREVIEW_MODE;

    if (is_charging)
    {
        Serial.println("9. Device is charging - skipping network initialization");
        lv_label_set_text(status_label, "Tryb ładowania\n\nPomijanie WiFi/MQTT");
        lv_task_handler();
        rgb.setColor(50, 50, 0); // Yellow for charging mode
        delay(1500);
    }
    else if (PREVIEW_MODE)
    {
        Serial.println("9. Preview mode - skipping network initialization");
        lv_label_set_text(status_label, "Tryb podglądu\n\nPomijanie WiFi/MQTT");
        lv_task_handler();
        rgb.setColor(0, 0, 50); // Blue for preview mode
        delay(1500);
    }

    // ==================== NETWORK INITIALIZATION ====================
    if (!skip_network)
    {
        // Initialize WiFi and RTC
        Serial.println("9. Initializing WiFi...");
        Serial.print("   Device ID: ");
        Serial.println(MQTT_CLIENT_ID);
        Serial.print("   SSID: ");
        Serial.println(WIFI_SSID);
        lv_label_set_text(status_label, "Inicjalizacja...\n\nŁączenie z WiFi...");
        lv_task_handler();
        wifi.begin(WIFI_SSID, WIFI_PASSWORD);

        // Try to connect with timeout (10 seconds max)
        unsigned long wifi_start = millis();
        const unsigned long wifi_timeout = 10000; // 10 seconds
        int dots = 0;
        while (!wifi.isConnected() && (millis() - wifi_start < wifi_timeout))
        {
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
            lv_task_handler(); // Force screen update

            wifi.loop();
            delay(500);
            dots++;
        }

        if (wifi.isConnected())
        {
            Serial.println("10. WiFi connected!");
            lv_label_set_text(status_label, "Inicjalizacja...\n\nWiFi: Połączono!\n\nŁadowanie...");
            lv_task_handler();
            rgb.setColor(0, 50, 0); // Green for WiFi success
            delay(1000);
        }
        else
        {
            Serial.println("10. WiFi connection timeout - continuing without WiFi");
            lv_label_set_text(status_label, "Inicjalizacja...\n\nWiFi: Brak połączenia\n(kontynuacja bez WiFi)\n\nŁadowanie...");
            lv_task_handler();
            rgb.setColor(50, 0, 0); // Red for WiFi failure
            delay(2000);
        }

        Serial.println("11. Initializing RTC...");
        rtc.begin();
        rtc.setLocalTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
        Serial.println("12. RTC initialized!");
    }
    else
    {
        Serial.println("9-12. Skipped network initialization (charging or preview mode)");
    }

    // ==================== SETUP SCREENS ====================
    // Create all screens
    Serial.println("13. Creating screens...");
    lv_label_set_text(status_label, "Tworzenie ekranów...");
    lv_task_handler();
    rgb.setColor(0, 0, 0); // Reset RGB
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

    navigator.registerScreen(LOADING, loadingScreen);
    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    navigator.registerScreen(BEFORE_CONCERT__SPONSORS, sponsorsScreen); 
    navigator.registerScreen(APP_GUIDE, appGuideScreen);
    navigator.registerScreen(SLIDER_DEMO, sliderScreen);
    navigator.registerScreen(SLIDER_DEMO__MEASUREMENT, tensionScreen);
    navigator.registerScreen(TENSION_MEASUREMENT, tensionScreen);
    navigator.registerScreen(BEFORE_CONCERT__RESEARCH_FORM, researchFormScreen);
    navigator.registerScreen(CHARGING, chargingScreen);
    navigator.registerScreen(OVATION, ovationScreen);
    navigator.registerScreen(PIECE_ANNOUNCEMENT, pieceAnnouncementScreen);
    navigator.registerScreen(END_OF_CONCERT, endOfConcertScreen);
    navigator.registerScreen(END_OF_CONCERT__FEEDBACK_FORM, feedbackFormScreen);
    navigator.registerScreen(END_OF_CONCERT__FORM_FINISHED, formFinishedScreen);
    navigator.registerScreen(SPONSORS, sponsorsScreen);

    Serial.println("16. Screens registered!");

    // Register page callbacks
    navigator.registerPageCallbacks(BEFORE_CONCERT, 
                                   BeforeConcertPage::firstRender, 
                                   BeforeConcertPage::lastRender);
    navigator.registerPageCallbacks(LOADING,
                                   LoadingPage::firstRender,
                                   LoadingPage::lastRender);
    navigator.registerPageCallbacks(APP_GUIDE,
                                   AppGuidePage::firstRender,
                                   AppGuidePage::lastRender);
    navigator.registerPageCallbacks(TENSION_MEASUREMENT,
                                   TensionMeasurementPage::firstRender,
                                   TensionMeasurementPage::lastRender);
    navigator.registerPageCallbacks(SLIDER_DEMO__MEASUREMENT,
                                   TensionMeasurementPage::firstRender,
                                   TensionMeasurementPage::lastRender);
    navigator.registerPageCallbacks(END_OF_CONCERT,
                                   EndOfConcertPage::firstRender,
                                   EndOfConcertPage::lastRender);

    // ==================== INITIALIZE HTTP AND EVENT POLLING ====================
    if (!skip_network && wifi.isConnected())
    {
        Serial.println("17. Initializing HTTP adapter...");
        lv_label_set_text(status_label, "Inicjalizacja HTTP...");
        lv_task_handler();
        httpAdapter.begin();
        Serial.println("18. HTTP adapter initialized!");

        if (USE_MQTT) {
            // MQTT initialization (currently disabled due to TLS issues)
            Serial.println("19. Creating MQTT adapter...");
            MQTTConnectionType mqttConnType = MQTT_USE_WSS ? MQTTConnectionType::WSS : MQTTConnectionType::TCP;
            mqttAdapter = new MQTTAdapterESP(MQTT_SERVER, MQTT_PORT, MQTT_CLIENT_ID, mqttConnType);

            if (MQTT_USE_WSS) {
                mqttAdapter->setWebSocketPath(MQTT_WS_PATH);
            }

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
            mqttPageBridge->addPageMapping("SLIDER_DEMO__MEASUREMENT", SLIDER_DEMO__MEASUREMENT);
            mqttPageBridge->addPageMapping("TENSION_MEASUREMENT", TENSION_MEASUREMENT);
            mqttPageBridge->addPageMapping("OVATION", OVATION);
            mqttPageBridge->addPageMapping("PIECE_ANNOUNCEMENT", PIECE_ANNOUNCEMENT);
            mqttPageBridge->addPageMapping("END_OF_CONCERT", END_OF_CONCERT);
            mqttPageBridge->addPageMapping("END_OF_CONCERT__FEEDBACK_FORM", END_OF_CONCERT__FEEDBACK_FORM);
            mqttPageBridge->addPageMapping("END_OF_CONCERT__FORM_FINISHED", END_OF_CONCERT__FORM_FINISHED);

            mqttPageBridge->registerPayloadHandler(OVATION, OvationPage::setPayload);
            mqttPageBridge->registerPayloadHandler(PIECE_ANNOUNCEMENT, PieceAnnouncementPage::setPayload);
            mqttPageBridge->registerPayloadHandler(TENSION_MEASUREMENT, TensionMeasurementPage::setPayload);
            mqttPageBridge->registerPayloadHandler(SLIDER_DEMO__MEASUREMENT, TensionMeasurementPage::setPayload);

            Serial.println("24. MQTT configured!");
        } else {
            // HTTP Polling initialization
            Serial.println("19. Creating Event Poller (HTTP)...");
            lv_label_set_text(status_label, "Konfiguracja pollingu...");
            lv_task_handler();
            
            eventPoller = new EventPoller(EVENT_API_ENDPOINT, POLL_INTERVAL_MS);
            eventPoller->begin();
            
            // Register event callback
            eventPoller->onEvent([](const EventSchema& event) {
                Serial.println("=== New Event Received via HTTP Polling ===");
                Serial.print("Event Type: ");
                Serial.println(event.eventType.c_str());
                Serial.print("Label: ");
                Serial.println(event.label.c_str());
                Serial.print("Position: ");
                Serial.println(event.position);
                
                // Map event type to page ID
                PageID targetPage = LOADING;
                if (event.eventType == "SPONSORS") targetPage = SPONSORS;
                else if (event.eventType == "LOADING") targetPage = LOADING;
                else if (event.eventType == "BEFORE_CONCERT") targetPage = BEFORE_CONCERT;
                else if (event.eventType == "BEFORE_CONCERT__RESEARCH_FORM") targetPage = BEFORE_CONCERT__RESEARCH_FORM;
                else if (event.eventType == "APP_GUIDE") targetPage = APP_GUIDE;
                else if (event.eventType == "SLIDER_DEMO") targetPage = SLIDER_DEMO;
                else if (event.eventType == "SLIDER_DEMO__MEASUREMENT") {
                    targetPage = SLIDER_DEMO__MEASUREMENT;
                    TensionMeasurementPage::setPayload(event);
                }
                else if (event.eventType == "TENSION_MEASUREMENT") {
                    targetPage = TENSION_MEASUREMENT;
                    TensionMeasurementPage::setPayload(event);
                }
                else if (event.eventType == "OVATION") {
                    targetPage = OVATION;
                    OvationPage::setPayload(event);
                }
                else if (event.eventType == "PIECE_ANNOUNCEMENT") {
                    targetPage = PIECE_ANNOUNCEMENT;
                    PieceAnnouncementPage::setPayload(event);
                }
                else if (event.eventType == "END_OF_CONCERT") targetPage = END_OF_CONCERT;
                else if (event.eventType == "END_OF_CONCERT__FEEDBACK_FORM") targetPage = END_OF_CONCERT__FEEDBACK_FORM;
                else if (event.eventType == "END_OF_CONCERT__FORM_FINISHED") targetPage = END_OF_CONCERT__FORM_FINISHED;
                
                // Navigate to page
                navigator.showPage(targetPage);
                Serial.print("Navigated to page: ");
                Serial.println((int)targetPage);
            });
            
            Serial.println("20. Event Poller configured!");
        }
    }
    else
    {
        if (skip_network)
        {
            Serial.println("17. Skipping HTTP/MQTT initialization - network disabled (charging/preview mode)");
            lv_label_set_text(status_label, "Pomijanie HTTP/MQTT\n(tryb ładowania/podglądu)");
        }
        else
        {
            Serial.println("17. Skipping HTTP/MQTT initialization - no WiFi connection");
            lv_label_set_text(status_label, "Pomijanie HTTP/MQTT\n(brak WiFi)");
        }
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
    // If device is charging at startup, show charging page
    // Note: is_charging was already declared earlier in setup
    if (is_charging)
    {
        Serial.println("26. Device is charging - showing CHARGING page...");
        navigator.showPage(CHARGING);
        // Turn off RGB
        rgb.setColor(0, 0, 0);
        was_charging = true;
        Serial.println("27. CHARGING page displayed!");
    }
    else
    {
        // Show sponsors page initially
        Serial.println("26. Showing LOADING page...");
        navigator.showPage(LOADING);
        Serial.println("27. LOADING page displayed!");
    }

    Serial.println("================================");
    Serial.println("=== Setup Complete ===");
    Serial.println("Press Button A to cycle pages");
    Serial.println("================================");
}

void loop()
{
    static unsigned long lastPrint = 0;
    static int counter = 0;

    M5.update();
    lv_task_handler(); // Handle LVGL tasks

    // ==================== NETWORK ADAPTERS ====================
    // Always call wifi.loop() to maintain connection
    wifi.loop();

    // Call HTTP adapter loop
    if (wifi.isConnected()) {
        httpAdapter.loop();
        
        // Call event poller if using HTTP polling
        if (eventPoller != nullptr) {
            eventPoller->loop();
        }
        
        // Call MQTT adapters if using MQTT
        if (mqttAdapter != nullptr && mqttPageBridge != nullptr) {
            mqttAdapter->loop();
            mqttPageBridge->loop();
        }
    }

    // ==================== DEBUG MEMORY LOGGING ====================
    counter++;
    if (millis() - lastPrint > 5000)
    {
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
    if (SHOW_CHARGING)
    {
        bool is_charging = M5.Power.isCharging();

        // If charging state changed to charging, switch to charging page
        if (is_charging && !was_charging)
        {
            Serial.println("Device started charging - showing CHARGING page");
            page_before_charging = navigator.getCurrentPage();
            navigator.showPage(CHARGING);
            rgb.setColor(0, 0, 0); // Turn off RGB
            was_charging = true;
        }
        // If unplugged from charging, restart device to reinitialize WiFi/MQTT
        else if (!is_charging && was_charging)
        {
            Serial.println("Device unplugged from charging - restarting to initialize WiFi/MQTT...");
            delay(500);
            ESP.restart(); // Restart the device
        }

        // Update battery display every 2 seconds when on charging page
        if (navigator.getCurrentPage() == CHARGING)
        {
            if (millis() - last_battery_update > 2000)
            {
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

    // ==================== TENSION MEASUREMENT PAGE LOGIC ====================

    // Update tension page with encoder value (with momentum physics)
    if (navigator.getCurrentPage() == TENSION_MEASUREMENT || navigator.getCurrentPage() == SLIDER_DEMO__MEASUREMENT)
    {
        int encoderValue = hmi.getEncoderValue();
        int rawDiff = encoderValue - lastEncoderValue;
        int delta = rawDiff; // use raw diff for proportional response

        // Throttled debug print to observe encoder behavior
        static unsigned long lastEncLog = 0;
        if (millis() - lastEncLog > 200)
        {
            Serial.printf("ENCODER raw=%d last=%d diff=%d delta=%d\n", encoderValue, lastEncoderValue, rawDiff, delta);
            lastEncLog = millis();
        }

        if (delta != 0)
        {
            lastEncoderValue += delta;
            TensionMeasurementPage::handleEncoder(delta);

            // Simulate buffer filling (kept for compatibility; label is commented out)
            tensionBufferCount++;
            if (tensionBufferCount > 200)
                tensionBufferCount = 0;
            TensionMeasurementPage::updateBuffer(tensionBufferCount, 200);
        }
    }

    // Small delay to avoid busy looping
    delay(5);
}
