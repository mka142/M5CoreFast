#include "PageNavigator.h"

// Forward declarations
extern void sliderDemoPageCleanup();
extern void sliderDemoPageResume();

void PageNavigator::registerScreen(PageID id, lv_obj_t *screen) {
    screens[id] = screen;
}

void PageNavigator::showPage(PageID id) {
    auto it = screens.find(id);
    if (it != screens.end()) {
        // Clean up previous page if it was SliderDemo
        if (currentPageID == SLIDER_DEMO) {
            sliderDemoPageCleanup();
        }
        
        currentScreen = it->second;
        currentPageID = id;
        lv_screen_load(currentScreen);
        
        // Resume new page if it's SliderDemo
        if (id == SLIDER_DEMO) {
            sliderDemoPageResume();
        }
    }
}
