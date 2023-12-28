#ifndef AFFICHEURS
#define AFFICHEURS

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>

class Afficheurs : public Fl_Group
{
public:
    Afficheurs(int x, int y, int w, int h, const char *l = nullptr);
    void draw();

private:
    Fl_Button **buttons;

};

#endif