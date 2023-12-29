#ifndef AFFICHEURS
#define AFFICHEURS

#include <FL/Fl_Scroll.H>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include "../store.hpp"
#include <list>

class DragButton : public Fl_Button
{
public:
    DragButton(int x, int y, int w, int h, const char *l = nullptr) : Fl_Button(x, y, w, h, l), last_y(0) {}
    int handle(int event) override
    {
        switch (event)
        {
        case FL_PUSH:
            last_y = Fl::event_y();
            return Fl_Button::handle(event);
        case FL_DRAG:
        {

            int dy = Fl::event_y() - last_y;
            Fl_Scroll *parent_ = (Fl_Scroll *)parent();

            int x = (parent_)->xposition();
            int y = (parent_)->yposition();
            last_y = Fl::event_y();
            int new_y = y - dy;
            if (max_size < new_y && dy < 0)
            {
                (parent_)->scroll_to(x, max_size);
                return 1;
            }

            if (new_y < 0 && dy > 0)
            {
                (parent_)->scroll_to(x, 0);
                return 1;
            }
            (parent_)->scroll_to(x, new_y);
            return 1;
        }
        default:
            return Fl_Button::handle(event);
        }
    }
    void set_max_size(int max_size)
    {
        this->max_size = max_size;
    }

private:
    int last_y;
    bool cache;
    int max_size;
};

class Afficheurs : public Fl_Group
{
public:
    Afficheurs(int x, int y, int w, int h, Store *store, const char *l = nullptr);
    ~Afficheurs();

    // copy from ordibus should be wrapped in own class to prevent WET code
    void add_number(int id);
    char *get_afficheur_id() const;
    void pop_afficheur_id();
    //
    void find_afficheur();
    void free_popup();
    void change_active(size_t idx);
    void draw();

private:
    // popup
    bool popup;
    Fl_Scroll *scroller;
    std::list<AfficheurData *> afficheurs;
    std::list<DragButton *> afficheur_btn;
    Fl_Button *quit;
    Fl_Button *add_aff;
    Fl_Button *replace_aff;
    size_t active_button;

    // afficheur
    Fl_Button **buttons;
    std::list<int> afficheur_id;
    Store *store;
};
typedef struct PassingVal_T
{
    void *ptr; //<- work around that'll store size_t as void* don't free it !!
    Afficheurs *parent;
} PassingVal;
#endif