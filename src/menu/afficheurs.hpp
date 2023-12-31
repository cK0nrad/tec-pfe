#ifndef AFFICHEURS
#define AFFICHEURS

#include <FL/Fl_Scroll.H>
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <list>

class Store;
class AfficheurData;

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
    void draw();
    void pop_afficheur_id();
    void list_afficheurs();
    void reset_girouette();
    void add_number(int id);
    void free_popup();
    void find_afficheur();
    void change_active(size_t idx);
    void push_afficheur();
    void replace_afficheur();
    void delete_afficheur();
    // copy from ordibus should be wrapped in own class to prevent WET code
private:
    char *get_afficheur_id() const;
    void free_search_popup();
    void free_list_popup();
    // popup (list afficheurs)
    bool popup_list;
    Fl_Button *delete_btn;

    // popup (search)
    bool popup;
    std::list<AfficheurData *> *afficheurs;
    std::list<DragButton *> *afficheur_btn;
    Fl_Scroll *scroller;
    Fl_Button *quit;
    Fl_Button *add_aff;
    Fl_Button *replace_aff;
    size_t active_button;

    // afficheur
    Fl_Button **buttons;
    std::list<int> *afficheur_id;
    Store *store;


    Fl_Image *loupe_img;
    Fl_Image *info_img;
};
typedef struct PassingVal_T
{
    void *ptr;
    Afficheurs *parent;
} PassingVal;
#endif