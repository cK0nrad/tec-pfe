#ifndef MAINTENANCE
#define MAINTENANCE

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

class Maintenance : public Fl_Group
{
public:
    Maintenance(int x, int y, int w, int h, const char *l = nullptr);
    void draw();
};

#endif