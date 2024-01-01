#include "afficheurs.hpp"
#include "../sqlite/afficheur_data.hpp"
#include "../sqlite/trip_data.hpp"
#include "../sqlite/request_manager.hpp"
#include "../layout.hpp"
#include "../store.hpp"

#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_BMP_Image.H>
#include <FL/Fl_Scroll.H>

#define BUTTON_SIZE 75
#define AFFICHEUR_HARD_LIMIT 15
#define POPUP_WIDTH WIDTH / 3
#define POPUP_HEIGHT 130

static const int AFFICEHURS_KEYPADS[15] = {0, 1, 2, 3, 4, 10, 5, 6, 7, 8, 9, 11, 12, 13, 14};
static const char *AFFICEHURS_KEYPADS_LABEL[12] = {"0", "1", "2", "3", "4", "←", "5", "6", "7", "8", "9", ""};
static int max_vertical_scroll(Fl_Scroll *scroller);

static void delete_btn_cb(Fl_Widget *, void *data)
{
    Afficheurs *aff = (Afficheurs *)data;
    aff->delete_afficheur();
}

void Afficheurs::delete_afficheur()
{
    store->pop_afficheur_id(active_button);
    free_popup();
}

static int max_vertical_scroll(Fl_Scroll *scroller)
{
    int max_y = 0;
    for (int i = 0; i < scroller->children(); ++i)
    {
        Fl_Widget *w = scroller->child(i);
        int y_extent = w->y() + w->h();
        if (y_extent > max_y)
        {
            max_y = y_extent;
        }
    }
    return max_y - scroller->h();
}

static void touch_button(Fl_Widget *widget, void *data)
{
    int button_number = *(int *)data;
    Afficheurs *afficheur = (Afficheurs *)widget->parent();

    switch (button_number)
    {
    case 10:
        afficheur->pop_afficheur_id();
        return;
    case 11:
        return;
    case 12:
        afficheur->list_afficheurs();
        return;
    case 13:
        afficheur->reset_girouette();
        return;
    case 14:
        afficheur->find_afficheur();
        return;
    default:
        break;
    }

    afficheur->add_number(button_number);
}

static void quit_popup(Fl_Widget *, void *parent)
{
    Afficheurs *afficheur = (Afficheurs *)parent;
    afficheur->free_popup();
}

static void change_selection(Fl_Widget *, void *val)
{
    PassingVal *ptr = (PassingVal *)val;
    ((Afficheurs *)(ptr->parent))->change_active((size_t)ptr->ptr);
}

static void push_girouette_cb(Fl_Widget *, void *val)
{
    Afficheurs *ptr = (Afficheurs *)val;
    ptr->push_afficheur();
}

static void replace_aff_cb(Fl_Widget *, void *val)
{
    Afficheurs *ptr = (Afficheurs *)val;
    ptr->replace_afficheur();
}

void Afficheurs::reset_girouette()
{
    store->reset_girouette();
}

void Afficheurs::list_afficheurs()
{
    if (popup_list || popup)
        return;

    for (size_t a = 0; a < (10 + 2 + 3); a++)
    {
        buttons[a]->deactivate();
    }

    size_t middle_point_x = (size_t)(WIDTH * 0.5);
    size_t middle_point_y = (size_t)(HEIGHT * 0.5);

    size_t y_position = TABS_HEIGHT;

    DragButton *new_button = nullptr;
    size_t idx = 0;

    std::list<AfficheurData *> *afficheurs = store->get_girouettes();
    for (auto a : *afficheurs)
    {
        char *line = a->get_line_formatter();
        new_button = new DragButton(
            middle_point_x - 2 * (int)(POPUP_WIDTH * .8 * .5),
            middle_point_y - POPUP_HEIGHT + y_position,
            2 * (int)(POPUP_WIDTH * .8),
            50, line);

        new_button->box(FL_BORDER_BOX);
        new_button->selection_color(FL_DARK_BLUE);

        PassingVal *val = (PassingVal *)malloc(sizeof(PassingVal));
        if (!val)
            throw std::bad_alloc();
        val->parent = this;
        val->ptr = (void *)idx++;

        new_button->callback(change_selection, (void *)val);
        scroller->add(new_button);
        afficheur_btn->push_back(new_button);
        y_position += 50;
        delete a; // not using a anymore
    }
    delete afficheurs;

    int max_scroll = max_vertical_scroll(scroller);

    for (auto a : *afficheur_btn)
    {
        a->set_max_size(max_scroll);
    }
    scroller->init_sizes();
    scroller->show();
    delete_btn->show();
    quit->show();
    popup_list = !popup_list;
    redraw();
}

void Afficheurs::replace_afficheur()
{
    if (!popup)
        return;

    if (!afficheurs || afficheurs->size() == 0)
        return;

    AfficheurData *afficheur = (*std::next(afficheurs->begin(), active_button));
    store->replace_girouette(afficheur);
    free_popup();
}

void Afficheurs::push_afficheur()
{
    if (!popup)
        return;

    if (!afficheurs || afficheurs->size() == 0)
        return;

    AfficheurData *afficheur = (*std::next(afficheurs->begin(), active_button));
    store->push_girouette(afficheur);
    free_popup();
}

void Afficheurs::change_active(size_t idx)
{
    if ((!popup && !popup_list) || idx > afficheur_btn->size() || idx == active_button)
        return;
    active_button = idx;
    redraw();
}

void Afficheurs::free_popup()
{
    active_button = 0;
    if (popup)
        free_search_popup();
    else if (popup_list)
        free_list_popup();
}
void Afficheurs::free_list_popup()
{
    popup_list = false;
    quit->hide();
    scroller->hide();
    delete_btn->hide();
    for (size_t a = 0; a < (10 + 2 + 3); a++)
        buttons[a]->activate();

    for (auto a : *afficheur_btn)
    {
        free(a->user_data());
        scroller->remove(a);
    }

    for (auto a : *afficheur_btn)
        delete a;

    afficheur_btn->clear();

    redraw();
}

void Afficheurs::free_search_popup()
{
    // free our afficheurs list
    for (auto a : *afficheur_btn)
    {
        free(a->user_data());
        scroller->remove(a);
    }

    for (auto a : *afficheurs)
        delete a;

    afficheurs->clear();
    afficheur_btn->clear();

    for (size_t a = 0; a < (10 + 2 + 3); a++)
        buttons[a]->activate();
    popup = false;
    scroller->hide();
    quit->hide();
    replace_aff->hide();
    add_aff->hide();
    redraw();
}

void Afficheurs::find_afficheur()
{
    if (popup || popup_list)
        return;

    active_button = 0;
    for (size_t a = 0; a < (10 + 2 + 3); a++)
    {
        buttons[a]->deactivate();
    }

    const char *id = get_afficheur_id();
    if (afficheurs)
    {
        for (auto a : *afficheurs)
            delete a;
        afficheurs->clear();
        delete afficheurs;
    }
    afficheurs = store->get_request_manager()->get_afficheur(id);
    free((void *)id);

    size_t middle_point_x = (size_t)(WIDTH * 0.5);
    size_t middle_point_y = (size_t)(HEIGHT * 0.5);

    size_t y_position = TABS_HEIGHT;
    size_t idx = 0;
    for (auto a : *afficheurs)
    {
        char *line = a->get_line_formatter();
        DragButton *new_button = new DragButton(middle_point_x - 2 * (int)(POPUP_WIDTH * .8 * .5), middle_point_y - POPUP_HEIGHT + y_position, 2 * (int)(POPUP_WIDTH * .8), 50, line);
        new_button->box(FL_BORDER_BOX);
        new_button->selection_color(FL_DARK_BLUE);

        PassingVal *val = (PassingVal *)malloc(sizeof(PassingVal));
        if (!val)
            throw std::bad_alloc();
        val->parent = this;
        val->ptr = (void *)idx++;

        new_button->callback(change_selection, (void *)val);
        scroller->add(new_button);
        afficheur_btn->push_back(new_button);
        y_position += 50;
    }

    int max_scroll = max_vertical_scroll(scroller);

    for (auto a : *afficheur_btn)
    {
        a->set_max_size(max_scroll);
    }

    scroller->init_sizes();
    scroller->show();
    quit->show();
    replace_aff->show();
    add_aff->show();
    popup = !popup;
    redraw();
}

Afficheurs::Afficheurs(int x, int y, int w, int h, Store *store, const char *l)
    : Fl_Group(x, y, w, h, l),
      popup_list(false),
      popup(false),
      afficheurs(new std::list<AfficheurData *>()),
      afficheur_btn(new std::list<DragButton *>()),
      active_button(0),
      afficheur_id(new std::list<int>()),
      store(store)
{
    begin();

    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    int middle = (int)(WIDTH * 0.5 * .25);

    loupe_img = new Fl_BMP_Image("./src/assets/loupe.bmp");
    info_img = new Fl_BMP_Image("./src/assets/info.bmp");

    size_t pos = 0;
    buttons = (Fl_Button **)malloc(sizeof(Fl_Button *) * (10 + 2 + 3));
    if (!buttons)
        throw std::bad_alloc();

    Fl_Button *button = nullptr;

    size_t middle_point_x = (size_t)(WIDTH * 0.5);
    size_t middle_point_y = (size_t)(HEIGHT * 0.5);

    size_t offset_y = h + y - 2 * TABS_HEIGHT - BUTTON_SIZE;
    size_t offset_x = (int)((WIDTH * 0.5) - (3 * BUTTON_SIZE));
    size_t offset_init_x = offset_x;
    for (size_t b = 0; b < 2; b++)
    {
        for (size_t a = 0; a < 6; a++)
        {
            button = new Fl_Button(offset_x, offset_y, BUTTON_SIZE, BUTTON_SIZE, AFFICEHURS_KEYPADS_LABEL[pos]);
            button->callback(touch_button, (void *)&AFFICEHURS_KEYPADS[pos]);
            buttons[pos++] = button;
            button->labelsize(50);
            offset_x += BUTTON_SIZE;
        }
        offset_x = offset_init_x;
        offset_y += BUTTON_SIZE;
    }

    buttons[5]->labelcolor(FL_RED);
    buttons[11]->labelcolor(FL_GREEN);

    // Info ()
    button = new Fl_Button(middle, 6 * TABS_HEIGHT - 20, BUTTON_SIZE, BUTTON_SIZE, "");
    button->box(FL_FLAT_BOX);
    button->image(info_img);
    button->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    button->callback(touch_button, (void *)&AFFICEHURS_KEYPADS[12]);

    buttons[pos++] = button;

    // List current
    button = new Fl_Button(middle_point_x - BUTTON_SIZE, 6 * TABS_HEIGHT - 20, 2 * BUTTON_SIZE, BUTTON_SIZE, "RESET");
    button->box(FL_FLAT_BOX);
    button->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    button->labelsize(32);
    button->callback(touch_button, (void *)&AFFICEHURS_KEYPADS[13]);
    buttons[pos++] = button;

    // Search
    button = new Fl_Button(middle + (int)(WIDTH * 0.75) - BUTTON_SIZE, 6 * TABS_HEIGHT - 20, BUTTON_SIZE, BUTTON_SIZE, "");
    button->box(FL_FLAT_BOX);
    button->image(loupe_img);
    button->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    button->callback(touch_button, (void *)&AFFICEHURS_KEYPADS[14]);
    buttons[pos++] = button;

    end();

    scroller = new Fl_Scroll(middle_point_x - POPUP_WIDTH, middle_point_y - POPUP_HEIGHT + TABS_HEIGHT, 2 * POPUP_WIDTH, 2 * POPUP_HEIGHT);
    scroller->end();
    scroller->hide();

    quit = new Fl_Button(middle_point_x + POPUP_WIDTH - 69, middle_point_y - POPUP_HEIGHT, 71, TABS_HEIGHT, "X");
    quit->box(FL_FLAT_BOX);
    quit->labelsize(20);
    quit->callback(quit_popup, this);
    quit->hide();

    add_aff = new Fl_Button(middle_point_x - POPUP_WIDTH - 1, middle_point_y + POPUP_HEIGHT + TABS_HEIGHT + 1, POPUP_WIDTH / 2, TABS_HEIGHT, "Ajouter");
    add_aff->labelsize(20);
    add_aff->box(FL_BORDER_BOX);
    add_aff->callback(push_girouette_cb, this);
    add_aff->hide();

    delete_btn = new Fl_Button(middle_point_x - POPUP_WIDTH - 1, middle_point_y + POPUP_HEIGHT + TABS_HEIGHT + 1, POPUP_WIDTH / 2, TABS_HEIGHT, "Supprimer");
    delete_btn->labelsize(20);
    delete_btn->box(FL_BORDER_BOX);
    delete_btn->callback(delete_btn_cb, this);
    delete_btn->hide();

    replace_aff = new Fl_Button(middle_point_x + POPUP_WIDTH - (POPUP_WIDTH / 2) + 3, middle_point_y + POPUP_HEIGHT + TABS_HEIGHT + 1, POPUP_WIDTH / 2, TABS_HEIGHT, "Remplacer");
    replace_aff->labelsize(20);
    replace_aff->box(FL_BORDER_BOX);
    replace_aff->callback(replace_aff_cb, this);
    replace_aff->hide();
}

Afficheurs::~Afficheurs()
{
    // FLTK will handle the deletion of the buttons when window is deleted
    for (size_t a = 0; a < (10 + 2 + 3); a++)
    {
        Fl::delete_widget(buttons[a]);
    }
    delete afficheur_id;

    if (afficheurs && afficheurs->size() > 0)
    {
        for (auto a : *afficheurs)
            delete a;
        afficheurs->clear();
    }
    delete afficheurs;

    if (afficheur_btn && afficheur_btn->size() > 0)
    {
        for (auto a : *afficheur_btn)
            Fl::delete_widget(a);
        afficheur_btn->clear();
    }
    delete afficheur_btn;

    Fl::delete_widget(scroller);
    Fl::delete_widget(quit);
    Fl::delete_widget(add_aff);
    Fl::delete_widget(replace_aff);

    delete loupe_img;
    delete info_img;
    // loupe_img->release();
    // info_img->release();
    free(buttons);
}
/////////
// DUPE //
/////////
void Afficheurs::add_number(int id)
{
    if (popup)
        return;

    if (afficheur_id->size() >= AFFICHEUR_HARD_LIMIT)
        return;

    if (id < 0 || id > 9)
        return;

    damage(0x90);
    afficheur_id->push_back(id);
}

char *Afficheurs::get_afficheur_id() const
{
    char *buffer = (char *)malloc((sizeof(char)) * (afficheur_id->size() + 1));
    if (!buffer)
        throw std::bad_alloc();
    int i = 0;
    for (int num : *afficheur_id)
    {
        buffer[i++] = '0' + static_cast<char>(num);
    }
    buffer[i] = '\0';
    return buffer;
}

void Afficheurs::pop_afficheur_id()
{
    if (afficheur_id->size() == 0)
    {
        redraw();
        return;
    }
    afficheur_id->pop_back();
    damage(0x90);
}

/////////
// DUPE//
/////////
void Afficheurs::draw()
{
    Fl_Group::draw();
    int middle = (int)(WIDTH * 0.5 * .25);
    fl_rectf(middle, 4 * TABS_HEIGHT + 10, (int)(WIDTH * 0.75), TABS_HEIGHT, FL_BLACK);
    fl_rect(middle, 4 * TABS_HEIGHT + 10, (int)(WIDTH * 0.75), TABS_HEIGHT, FL_DARK_BLUE);

    fl_rect(middle - 3, 2 * TABS_HEIGHT + 13, (int)(WIDTH * 0.75) + 6, 3 * TABS_HEIGHT, FL_BLACK);
    fl_rectf(middle, 2 * TABS_HEIGHT, 95, 20, SECONDARY);
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 20);

    const char *id = get_afficheur_id();
    fl_draw(id, middle, 4 * TABS_HEIGHT + 10, (int)(WIDTH * 0.75), TABS_HEIGHT, FL_ALIGN_CENTER);
    free((void *)id);

    fl_font(FL_HELVETICA, 16);
    fl_draw("Destination", middle + 1, 2 * TABS_HEIGHT + 4, 70, 20, FL_ALIGN_LEFT);

    fl_font(FL_HELVETICA_BOLD, 24);

    const char *line = store->get_current_girouette().get_formatter();
    fl_draw(line, middle - 3, 2 * TABS_HEIGHT + 13, (int)(WIDTH * 0.75) + 6, 3 * TABS_HEIGHT - 20, FL_ALIGN_CENTER);
    free((void *)line);

    if (popup_list && !popup)
    {
        size_t middle_point_x = (size_t)(WIDTH * 0.5);
        size_t middle_point_y = (size_t)(HEIGHT * 0.5);
        fl_rectf(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BACKGROUND_COLOR);
        fl_rect(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BLACK);
        size_t idx = 0;
        for (auto a : *afficheur_btn)
        {
            if (idx++ == active_button)
            {
                a->color(FL_BLUE);
                a->labelcolor(FL_WHITE);
            }
            else
            {
                a->color(FL_WHITE);
                a->labelcolor(FL_BLACK);
            }
        }
        quit->redraw();
        delete_btn->redraw();
        scroller->redraw();
    }

    if (popup && !popup_list)
    {
        size_t middle_point_x = (size_t)(WIDTH * 0.5);
        size_t middle_point_y = (size_t)(HEIGHT * 0.5);
        fl_rectf(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BACKGROUND_COLOR);
        fl_rect(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BLACK);

        size_t idx = 0;
        for (auto a : *afficheur_btn)
        {
            if (idx++ == active_button)
            {
                a->color(FL_BLUE);
                a->labelcolor(FL_WHITE);
            }
            else
            {
                a->color(FL_WHITE);
                a->labelcolor(FL_BLACK);
            }
        }
        if (afficheur_btn->size() == 0)
        {
            scroller->hide();
            scroller->redraw();
            quit->redraw();
            add_aff->redraw();
            replace_aff->redraw();
            fl_font(FL_HELVETICA_BOLD, 16);
            fl_draw("AUCUN RESULTAT", middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_ALIGN_CENTER);
            return;
        }
        else
        {
            quit->redraw();
            scroller->redraw();
            add_aff->redraw();
            replace_aff->redraw();
        }
    }
}
