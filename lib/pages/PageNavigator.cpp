#include "PageNavigator.h"

void PageNavigator::registerScreen(PageID id, lv_obj_t *screen) {
    screens[id] = screen;
}

void PageNavigator::showPage(PageID id) {
    auto it = screens.find(id);
    if (it != screens.end()) {
        currentScreen = it->second;
        currentPageID = id;
        lv_screen_load(currentScreen);
    }
}
