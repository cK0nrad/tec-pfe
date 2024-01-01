#ifndef ORDIBUS
#define ORDIBUS

#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <list>

class Store;

class Ordibus : public Fl_Group
{
public:
    Ordibus(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    ~Ordibus();
    void reset_service();
    void draw();
    void init_route();
    void pop_route_id();
    void set_error();
    void add_number(int id);

private:
    char *get_route_id() const;
    std::list<Fl_Button *> buttons;
    std::list<int> route_id;
    Store *store;
    bool error;
};

#endif