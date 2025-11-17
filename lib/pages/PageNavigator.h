#pragma once
#include <lvgl.h>
#include "PageID.h"
#include <map>

// Simple page navigator - just switches between LVGL screens
// No complex abstractions - KISS principle
class PageNavigator {
public:
    void registerScreen(PageID id, lv_obj_t *screen);
    void showPage(PageID id);
    PageID getCurrentPage() const { return currentPageID; }
    lv_obj_t* getCurrentScreen() const { return currentScreen; }
    
private:
    std::map<PageID, lv_obj_t*> screens;
    lv_obj_t *currentScreen = nullptr;
    PageID currentPageID = LOADING;
};
