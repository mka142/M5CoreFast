#pragma once
#include "../ui/PageID.h"

class IPageNavigator {
public:
    virtual void requestPageChange(PageID id) = 0;
    virtual ~IPageNavigator() {}
};
