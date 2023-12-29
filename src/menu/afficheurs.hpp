#ifndef AFFICHEURS
#define AFFICHEURS

#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include "../store.hpp"
#include <list>

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
    std::list<Fl_Button *> afficheur_btn;
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