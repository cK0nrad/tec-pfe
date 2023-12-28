#ifndef CURRENT_TIME
#define CURRENT_TIME

#include <FL/Fl_Box.H>
#include "../store.hpp"

class TECClock : public Fl_Box
{
public:
    TECClock(int X, int Y, int W, int H, Store *store, const char *L = nullptr);
    ~TECClock();
    void update();
    void draw();
private:
    Store *store;
};
#endif