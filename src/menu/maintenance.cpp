#include "maintenance.hpp"
#include "../layout.hpp"

#include <FL/Fl_Group.H>

Maintenance::Maintenance(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    end();
}

void Maintenance::draw()
{
    Fl_Group::draw();
}
