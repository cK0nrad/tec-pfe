#include "custom_group.hpp"
#include "../layout.hpp"
#include <cstdio>
#include <FL/Fl_Tabs.H>

TECGroup::TECGroup(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    selection_color(PRIMARY);
    labelcolor(FL_WHITE);
}