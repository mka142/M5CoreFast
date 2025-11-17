// ============================================================
// EXAMPLE: How to integrate SponsorsPage in main.cpp
// ============================================================

/*

1. ADD INCLUDES at top of main.cpp:

   #include "pages/SponsorsPage.h"

2. CREATE SCREEN in setup():

   // After other pages...
   lv_obj_t *sponsorsScreen = SponsorsPage::create();
   
3. REGISTER with navigator:

   navigator.registerScreen(SPONSORS, sponsorsScreen);
   
4. SHOW at beginning:

   navigator.showPage(SPONSORS);
   
5. OR show at end in loop():

   if (concertEnded) {
       navigator.showPage(SPONSORS);
   }

COMPLETE EXAMPLE:
-----------------

#include "pages/SponsorsPage.h"

void setup() {
    // ... M5, LVGL, adapters init ...
    
    // Create all screens
    lv_obj_t *loadingScreen = LoadingPage::create();
    lv_obj_t *sponsorsScreen = SponsorsPage::create();  // ← ADD THIS
    lv_obj_t *beforeConcertScreen = BeforeConcertPage::create();
    // ... other pages ...
    
    // Register
    navigator.registerScreen(LOADING, loadingScreen);
    navigator.registerScreen(SPONSORS, sponsorsScreen);  // ← ADD THIS
    navigator.registerScreen(BEFORE_CONCERT, beforeConcertScreen);
    // ... other pages ...
    
    // Show sponsors first
    navigator.showPage(SPONSORS);  // ← START HERE
    
    Serial.println("Setup complete - showing sponsors");
}

void loop() {
    M5.update();
    lv_task_handler();
    
    // Example: Button A cycles pages
    if (hmi.getButtonA() && !btnA_pressed) {
        btnA_pressed = true;
        
        PageID current = navigator.getCurrentPage();
        switch(current) {
            case SPONSORS:
                navigator.showPage(BEFORE_CONCERT);
                break;
            case BEFORE_CONCERT:
                navigator.showPage(SLIDER_DEMO);
                break;
            // ... etc ...
            case END_OF_CONCERT:
                navigator.showPage(SPONSORS);  // ← BACK TO SPONSORS
                break;
        }
    }
    if (!hmi.getButtonA()) btnA_pressed = false;
    
    delay(5);
}

MQTT INTEGRATION EXAMPLE:
--------------------------

void onMQTTEvent(const EventSchema& event) {
    if (event.eventType == "CONCERT_START") {
        navigator.showPage(SPONSORS);  // Show sponsors before concert
    }
    else if (event.eventType == "CONCERT_END") {
        navigator.showPage(SPONSORS);  // Show sponsors after concert
    }
    else if (event.eventType == "SKIP_SPONSORS") {
        navigator.showPage(BEFORE_CONCERT);  // Skip to next
    }
}

CUSTOMIZATION EXAMPLES:
-----------------------

// Different timing:
SponsorCarousel::start(5000, true);  // 5s per logo

// Sequential order:
SponsorCarousel::start(3000, false);  // Sequential, not random

// Stop manually:
if (hmi.getButtonB()) {
    SponsorCarousel::stop();
    navigator.showPage(BEFORE_CONCERT);
}

// Add custom logos (after running convert.sh with new images):
SponsorCarousel::addLogo(&logo_new_sponsor);

*/
