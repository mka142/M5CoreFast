#pragma once
#include <lvgl.h>
#include "PageID.h"
#include <map>
#include <functional>

// Simple page navigator - just switches between LVGL screens
// No complex abstractions - KISS principle
class PageNavigator {
public:
    void registerScreen(PageID id, lv_obj_t *screen);
    void registerPageCallbacks(PageID id, 
                              std::function<void()> firstRender = nullptr,
                              std::function<void()> lastRender = nullptr);
    void showPage(PageID id);
    PageID getCurrentPage() const { return currentPageID; }
    lv_obj_t* getCurrentScreen() const { return currentScreen; }
    
private:
    struct PageCallbacks {
        std::function<void()> firstRender;
        std::function<void()> lastRender;
    };
    
    std::map<PageID, lv_obj_t*> screens;
    std::map<PageID, PageCallbacks> callbacks;
    lv_obj_t *currentScreen = nullptr;
    PageID currentPageID = LOADING;
};
