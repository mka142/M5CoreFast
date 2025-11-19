#include "PageNavigator.h"

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
        
        // Call firstRender for new page if it exists
        auto newCallbacks = callbacks.find(id);
        if (newCallbacks != callbacks.end() && newCallbacks->second.firstRender) {
            newCallbacks->second.firstRender();
        }
    }
}
