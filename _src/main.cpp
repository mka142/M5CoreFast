#include <DisplayAdapter.h>
#include <TextAdapter.h>
#include <WiFiAdapter.h>
#include <RTCAdapter.h>
#include <HMIAdapter.h>
#include <RGBAdapter.h>
#include <MQTTAdapter.h>
#include <MQTTPageBridge.h>
#include <HTTPAdapter.h>

#include <M5CoreS3.h>

#include "pages/BeforeConcert.h"
#include "pages/OvationPage.h"
#include "pages/TensionMeasurementPage.h"
#include "pages/AppGuidePage.h"
#include "pages/SliderDemoPage.h"
#include "pages/ConcertStartPage.h"
#include "pages/PieceAnnouncementPage.h"
#include "pages/EndOfConcertPage.h"

#include <PageManager.h>

#define WIFI_SSID "SZOK"
#define WIFI_PASSWORD "uwagapodajehaslo"
#define NTP_TIMEZONE "UTC+2"
#define NTP_SERVER1 "0.pool.ntp.org"
#define NTP_SERVER2 "1.pool.ntp.org"
#define NTP_SERVER3 "2.pool.ntp.org"

// MQTT Configuration
#define MQTT_SERVER "server.device-manager.fast.knakitm.pl"
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
#define FORM_API_HOST "http://192.168.1.20:3001/api/forms/batch" //"https://server.device-manager.fast.knakitm.pl/api/forms/batch"

DisplayAdapter displayAdapter;
TextAdapter textAdapter(displayAdapter);
WiFiAdapter wifi;
RTCAdapter rtc;
HMIAdapter hmi;
RGBAdapter rgb;
PageManager pageManager;

// Pages
BeforeConcertPage beforeConcertPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
OvationPage ovationPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
AppGuidePage appGuidePage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
SliderDemoPage sliderDemoPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
ConcertStartPage concertStartPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
PieceAnnouncementPage pieceAnnouncementPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
EndOfConcertPage endOfConcertPage(displayAdapter, textAdapter, hmi, rgb, &pageManager);

// MQTT components - independent and loosely coupled
MQTTAdapter mqttAdapter(MQTT_SERVER, MQTT_PORT, MQTT_CLIENT_ID);
MQTTPageBridge mqttPageBridge(mqttAdapter, pageManager);

// HTTP adapter for API requests
HTTPAdapter httpAdapter;

// Tension measurement page (requires RTC and HTTP adapter)
TensionMeasurementPage tensionPage(displayAdapter, textAdapter, hmi, rgb, rtc, httpAdapter,
                                   FORM_API_HOST, MQTT_CLIENT_ID, &pageManager);

void setup()
{
    // Initialize Serial for debugging
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("Starting system...");
    Serial.print("Device ID: ");
    Serial.println(MQTT_CLIENT_ID);

    // This is required to initialize M5CoreS3 components
    CoreS3.begin();

    displayAdapter.begin();
    displayAdapter.configure();
    textAdapter.begin(displayAdapter);

    displayAdapter.clear();
    textAdapter.setFont(FontStyle::Normal, FontSize::Normal);

    rgb.begin(5, 10); // Pin 5, 10 LEDs
    hmi.begin();

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (!wifi.isConnected())
    {
        Serial.println("Connecting to WiFi...");
        wifi.loop();
        delay(100);
    }

    rtc.begin();
    // rtc.setLocalTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    textAdapter.drawText("System załadowany", TextAlignX::Left, TextAlignY::Top, FontStyle::Bold, FontSize::Normal);

    textAdapter.drawText("Zobacz HMI... (pokrętło)", TextAlignX::Right, TextAlignY::Center, FontStyle::Italic, FontSize::Small);

    displayAdapter.update();

    // Register pages
    pageManager.registerPage(BEFORE_CONCERT, &beforeConcertPage);
    pageManager.registerPage(OVATION, &ovationPage);
    pageManager.registerPage(TENSION_MEASUREMENT, &tensionPage);
    pageManager.registerPage(APP_GUIDE, &appGuidePage);
    pageManager.registerPage(SLIDER_DEMO, &sliderDemoPage);
    pageManager.registerPage(CONCERT_START, &concertStartPage);
    pageManager.registerPage(PIECE_ANNOUNCEMENT, &pieceAnnouncementPage);
    pageManager.registerPage(END_OF_CONCERT, &endOfConcertPage);

    // Initialize HTTP adapter
    httpAdapter.begin();

    // Initialize MQTT after WiFi is connected
    mqttAdapter.begin(MQTT_USERNAME, MQTT_PASSWORD);
    mqttAdapter.subscribeTo(MQTT_TOPIC_EVENTS);

    // Setup page mappings (eventType -> PageID)
    mqttPageBridge.begin();
    mqttPageBridge.addPageMapping("BEFORE_CONCERT", BEFORE_CONCERT);
    mqttPageBridge.addPageMapping("OVATION", OVATION);
    mqttPageBridge.addPageMapping("TENSION_MEASUREMENT", TENSION_MEASUREMENT);
    mqttPageBridge.addPageMapping("APP_GUIDE", APP_GUIDE);
    mqttPageBridge.addPageMapping("SLIDER_DEMO", SLIDER_DEMO);
    mqttPageBridge.addPageMapping("CONCERT_START", CONCERT_START);
    mqttPageBridge.addPageMapping("PIECE_ANNOUNCEMENT", PIECE_ANNOUNCEMENT);
    mqttPageBridge.addPageMapping("END_OF_CONCERT", END_OF_CONCERT);

    pageManager.requestPageChange(BEFORE_CONCERT); // Start on before concert page
}

void loop()
{
    displayAdapter.waitDisplay();
    wifi.loop();

    // Non-blocking MQTT processing
    mqttAdapter.loop();
    mqttPageBridge.loop();

    // Non-blocking HTTP processing
    httpAdapter.loop();

    pageManager.updatePageIfNeeded();
    Page *currentPage = pageManager.getCurrentPage();
    if (currentPage)
    {
        currentPage->handleInput();
        currentPage->render();
    }
    displayAdapter.update();
}

// int encoder = hmi.getEncoderValue();
//     bool btnA = hmi.getButtonA();
//     bool btnB = hmi.getButtonB();
//     bool btnS = hmi.getButtonS();

//     // if btnA or btnB pressed,

//     // Example: change RGB color based on encoder
//     rgb.setColor(encoder % 255, (encoder * 2) % 255, (encoder * 3) % 255);

//     // Example: display time
//     tm timeInfo = rtc.getTime();
//     char buf[32];
//     sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
//             timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

//     textAdapter.drawText(buf, TextAlignX::Left, TextAlignY::Bottom, FontStyle::Normal, FontSize::Normal);