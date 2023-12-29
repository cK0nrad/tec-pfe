#ifndef ORDIBUS
#define ORDIBUS

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <list>

class Ordibus : public Fl_Group
{
public:
    Ordibus(int x, int y, int w, int h, const char *l = nullptr);
    ~Ordibus();
    void draw();
    void add_number(int id);
    char *get_route_id() const;
    void pop_route_id();

private:
    std::list<Fl_Button *> buttons;
    std::list<int> route_id;
};

#endif