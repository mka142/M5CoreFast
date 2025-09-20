
#pragma once
#include <DisplayAdapter.h>
#include <TextAdapter.h>
#include <HMIAdapter.h>
#include <RGBAdapter.h>
#include "../ui/PageID.h"
#include "IPageNavigator.h"

class Page
{

public:
    Page(PageID id,
         DisplayAdapter &display,
         TextAdapter &text,
         HMIAdapter &hmi,
         RGBAdapter &rgb,
         IPageNavigator *navigator = nullptr)
        : id(id), display(display), text(text), hmi(hmi), rgb(rgb), navigator(navigator) {}

    virtual ~Page() {}

    PageID getID() const { return id; }

    void setNavigator(IPageNavigator *nav) { navigator = nav; }

    // Allow page to request a page change
    void requestPageChange(PageID id)
    {
        if (navigator)
            navigator->requestPageChange(id);
    }

    // Called once when the page is rendered for the first time after navigation
    virtual void firstRender() {}

    // Called once before the page is switched away
    virtual void lastRender() {}

    // Called every frame to render the page
    virtual void render() = 0;
    // Called every frame to handle input
    virtual void handleInput() = 0;

protected:
    PageID id;
    DisplayAdapter &display;
    TextAdapter &text;
    HMIAdapter &hmi;
    RGBAdapter &rgb;
    IPageNavigator *navigator;
};
