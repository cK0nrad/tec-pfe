#ifndef BILLETIQUE
#define BILLETIQUE

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>



class Billetique : public Fl_Group
{
public:
    Billetique(int x, int y, int w, int h, const char *l = nullptr);
    void draw();

private:
    Fl_Button **buttons;

};




#endif