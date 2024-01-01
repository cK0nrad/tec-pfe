#ifndef PILOTAUTO
#define PILOTAUTO

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_BMP_Image.H>

class Store;

class Pilotauto : public Fl_Group
{
public:
    Pilotauto(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    ~Pilotauto();
    void draw();
private:
    Store *store;
    Fl_Button *overwrite;
    Fl_Button *next;
    Fl_Button *prev;
    
    Fl_Image *next_img;
    Fl_Image *prev_img;
    
};

#endif