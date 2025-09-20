#include <DisplayAdapter.h>
#include <TextAdapter.h>
#include <WiFiAdapter.h>
#include <RTCAdapter.h>
#include <HMIAdapter.h>
#include <RGBAdapter.h>

#include <M5CoreS3.h>

#include "pages/HomePage.h"
#include "pages/TestPage2.h"
#include <PageManager.h>

#define WIFI_SSID "Orange_IoT_4040"
#define WIFI_PASSWORD "LMWWUWtFyqXKHQfuzU"
#define NTP_TIMEZONE "UTC+2"
#define NTP_SERVER1 "0.pool.ntp.org"
#define NTP_SERVER2 "1.pool.ntp.org"
#define NTP_SERVER3 "2.pool.ntp.org"

DisplayAdapter displayAdapter;
TextAdapter textAdapter(displayAdapter);
WiFiAdapter wifi;
RTCAdapter rtc;
HMIAdapter hmi;
RGBAdapter rgb;
PageManager pageManager;
HomePage homePage(displayAdapter, textAdapter, hmi, rgb, &pageManager);
TestPage2 testPage2(displayAdapter, textAdapter, hmi, rgb, &pageManager);

void setup()
{
    // Initialize Serial for debugging
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.println("Starting system...");

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
        wifi.loop();
        delay(100);
    }

    rtc.begin();
    rtc.setLocalTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

    textAdapter.drawText("System załadowany", TextAlignX::Left, TextAlignY::Top, FontStyle::Bold, FontSize::Normal);

    textAdapter.drawText("Zobacz HMI... (pokrętło)", TextAlignX::Right, TextAlignY::Center, FontStyle::Italic, FontSize::Small);

    displayAdapter.update();

    pageManager.registerPage(PAGE_HOME, &homePage);
    pageManager.registerPage(PAGE_INFO, &testPage2);
    pageManager.requestPageChange(PAGE_HOME); // Start on home page
}

void loop()
{
    displayAdapter.waitDisplay();
    wifi.loop();

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