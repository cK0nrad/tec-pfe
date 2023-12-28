#ifndef PILOTAUTO
#define PILOTAUTO

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include "../store.hpp"

class Pilotauto : public Fl_Group
{
public:
    Pilotauto(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    void draw();
private:
    Store *store;
};

#endif