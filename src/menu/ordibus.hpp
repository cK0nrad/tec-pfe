#ifndef ORDIBUS
#define ORDIBUS

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <list>
#include "../store.hpp"

class Ordibus : public Fl_Group
{
public:
    Ordibus(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    ~Ordibus();
    void draw();
    void add_number(int id);
    char *get_route_id() const;
    void pop_route_id();
    void init_route();

private:
    std::list<Fl_Button *> buttons;
    std::list<int> route_id;
    Store *store;
};

#endif