#include "PageNavigator.h"
#include <Arduino.h>

void PageNavigator::registerScreen(PageID id, lv_obj_t *screen) {
    screens[id] = screen;
}

void PageNavigator::registerPageCallbacks(PageID id, 
                                        std::function<void()> firstRender,
                                        std::function<void()> lastRender) {
    callbacks[id] = {firstRender, lastRender};
}

void PageNavigator::showPage(PageID id) {
    auto it = screens.find(id);
    if (it != screens.end()) {
        Serial.printf("PageNavigator: showPage requested -> %d, current -> %d\n", (int)id, (int)currentPageID);
        // Call lastRender for current page if it exists
        if (currentPageID != id) {
            auto currentCallbacks = callbacks.find(currentPageID);
            if (currentCallbacks != callbacks.end() && currentCallbacks->second.lastRender) {
                currentCallbacks->second.lastRender();
            }
        }

        currentScreen = it->second;
        currentPageID = id;
        lv_screen_load(currentScreen);
        Serial.printf("PageNavigator: lv_screen_load called for -> %d\n", (int)id);

        // Call firstRender for new page if it exists
        auto newCallbacks = callbacks.find(id);
        if (newCallbacks != callbacks.end() && newCallbacks->second.firstRender) {
            newCallbacks->second.firstRender();
        }
        Serial.printf("PageNavigator: showPage completed -> %d\n", (int)id);
    }
}
