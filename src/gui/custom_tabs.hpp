#ifndef TEC_TABS
#define TEC_TABS

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include "custom_group.hpp"

class Store;

class TECTabs : public TECGroup
{
public:
    TECTabs(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    ~TECTabs();
    void draw();
    void change_tab(int tab);

private:
    int active_tab = 0;
    Fl_Box **active_box;
    Store *store;
};
#endif