#pragma once
#include "../ui/PageID.h"
#include <Page.h>
#include <map>

class PageManager : public IPageNavigator {
public:
    PageManager() : currentPage(nullptr), requestedPageID(BEFORE_CONCERT), lastPage(nullptr) {}

    void registerPage(PageID id, Page* page) {
        pages[id] = page;
    }

    void requestPageChange(PageID id) {
        requestedPageID = id;
    }

    void updatePageIfNeeded() {
        if (currentPage == nullptr || requestedPageID != currentPage->getID()) {
            // Call lastRender on the old page
            if (currentPage) {
                currentPage->lastRender();
            }
            auto it = pages.find(requestedPageID);
            if (it != pages.end()) {
                currentPage = it->second;
                // Call firstRender on the new page
                currentPage->firstRender();
            }
        }
    }

    Page* getCurrentPage() {
        return currentPage;
    }
    
    Page* getPage(PageID id) {
        auto it = pages.find(id);
        if (it != pages.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    std::map<PageID, Page*> pages;
    Page* currentPage;
    PageID requestedPageID;
    Page* lastPage;
};
