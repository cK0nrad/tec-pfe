#include "afficheurs.hpp"
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

static const int AFFICEHURS_KEYPADS[13] = {0, 1, 2, 3, 4, 10, 5, 6, 7, 8, 9, 11, 12};
static const char *AFFICEHURS_KEYPADS_LABEL[12] = {"0", "1", "2", "3", "4", "←", "5", "6", "7", "8", "9", ""};

int max_vertical_scroll(Fl_Scroll *scroller)
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
    {
        afficheur->find_afficheur();
        return;
    }
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

void Afficheurs::change_active(size_t idx)
{
    if (!popup || idx > afficheur_btn.size() || idx == active_button)
        return;
    active_button = idx;
    redraw();
}

void Afficheurs::free_popup()
{
    if (!popup)
        return;

    // free our afficheurs list
    for (auto a : afficheur_btn)
    {
        free(a->user_data());
        scroller->remove(a);
    }

    for (auto a : afficheurs)
        delete a;

    afficheurs.clear();
    afficheur_btn.clear();

    for (size_t a = 0; a < (10 + 2 + 2); a++)
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
    if (popup)
        return;

    active_button = 0;
    for (size_t a = 0; a < (10 + 2 + 2); a++)
    {
        buttons[a]->deactivate();
    }

    const char *id = get_afficheur_id();
    afficheurs = store->get_request_manager()->get_afficheur(id);
    free((void *)id);

    size_t middle_point_x = (size_t)(WIDTH * 0.5);
    size_t middle_point_y = (size_t)(HEIGHT * 0.5);

    size_t y_position = TABS_HEIGHT;
    size_t idx = 0;
    for (auto a : afficheurs)
    {
        size_t id_size = strlen(a->get_id());
        size_t line_size = strlen(a->get_line());
        size_t text_size = strlen(a->get_text());

        //[id]: [text]\0
        char *line = (char *)malloc(sizeof(char) * (1 + id_size + 2 + line_size + 2 + text_size + 1));
        strcpy(line, "[");
        strcat(line, a->get_id());
        strcat(line, "] ");
        strcat(line, a->get_line());
        strcat(line, ": ");
        strcat(line, a->get_text());

        DragButton *new_button = new DragButton(middle_point_x - 2 * (int)(POPUP_WIDTH * .8 * .5), middle_point_y - POPUP_HEIGHT + y_position, 2 * (int)(POPUP_WIDTH * .8), 50, line);
        new_button->box(FL_BORDER_BOX);
        new_button->selection_color(FL_DARK_BLUE);

        PassingVal *val = (PassingVal *)malloc(sizeof(PassingVal));
        val->parent = this;
        val->ptr = (void *)idx++;

        // when draging;

        new_button->callback(change_selection, (void *)val);

        scroller->add(new_button);
        afficheur_btn.push_back(new_button);
        y_position += 50;
    }

    int max_scroll = max_vertical_scroll(scroller);

    for (auto a : afficheur_btn)
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
    : Fl_Group(x, y, w, h, l), popup(false), afficheur_btn(std::list<DragButton *>()), active_button(0), store(store)
{
    begin();
    afficheur_id = std::list<int>();

    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    int middle = (int)(WIDTH * 0.5 * .25);

    Fl_Image *loupe_img = new Fl_BMP_Image("./src/assets/loupe.bmp");
    Fl_Image *info_img = new Fl_BMP_Image("./src/assets/info.bmp");

    size_t pos = 0;
    buttons = (Fl_Button **)malloc(sizeof(Fl_Button *) * (10 + 2 + 2));
    Fl_Button *button = nullptr;

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
    buttons[pos++] = button;

    // Search
    button = new Fl_Button(middle + (int)(WIDTH * 0.75) - BUTTON_SIZE, 6 * TABS_HEIGHT - 20, BUTTON_SIZE, BUTTON_SIZE, "");
    button->box(FL_FLAT_BOX);
    button->image(loupe_img);
    button->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
    button->callback(touch_button, (void *)&AFFICEHURS_KEYPADS[12]);
    buttons[pos++] = button;

    end();
    size_t middle_point_x = (size_t)(WIDTH * 0.5);
    size_t middle_point_y = (size_t)(HEIGHT * 0.5);
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
    add_aff->hide();

    replace_aff = new Fl_Button(middle_point_x + POPUP_WIDTH - (POPUP_WIDTH / 2) + 3, middle_point_y + POPUP_HEIGHT + TABS_HEIGHT + 1, POPUP_WIDTH / 2, TABS_HEIGHT, "Remplacer");
    replace_aff->labelsize(20);
    replace_aff->box(FL_BORDER_BOX);
    replace_aff->hide();
}

Afficheurs::~Afficheurs()
{
    // FLTK will handle the deletion of the buttons when window is deleted
    for (size_t a = 0; a < (10 + 2 + 2); a++)
    {
        delete buttons[a];
    }
    free(buttons);
}
/////////
// DUPE //
/////////
void Afficheurs::add_number(int id)
{
    if (popup)
        return;

    if (afficheur_id.size() >= AFFICHEUR_HARD_LIMIT)
        return;

    if (id < 0 || id > 9)
        return;

    damage(0x90);
    afficheur_id.push_back(id);
}

char *Afficheurs::get_afficheur_id() const
{
    char *buffer = (char *)malloc((sizeof(char)) * (afficheur_id.size() + 1));
    int i = 0;
    for (int num : afficheur_id)
    {
        buffer[i++] = '0' + static_cast<char>(num);
    }
    buffer[i] = '\0';
    return buffer;
}

void Afficheurs::pop_afficheur_id()
{
    if (afficheur_id.size() == 0)
    {
        redraw();
        return;
    }
    afficheur_id.pop_back();
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
    fl_draw("29 - RENTRE AU DEPOT", middle - 3, 2 * TABS_HEIGHT + 13, (int)(WIDTH * 0.75) + 6, 3 * TABS_HEIGHT - 20, FL_ALIGN_CENTER);

    if (popup)
    {

        size_t middle_point_x = (size_t)(WIDTH * 0.5);
        size_t middle_point_y = (size_t)(HEIGHT * 0.5);
        fl_rectf(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BACKGROUND_COLOR);
        fl_rect(middle_point_x - POPUP_WIDTH - 1, middle_point_y - POPUP_HEIGHT - 1, 2 * (POPUP_WIDTH + 2), 2 * (POPUP_HEIGHT + 1) + 2 * TABS_HEIGHT, FL_BLACK);

        size_t idx = 0;
        for (auto a : afficheur_btn)
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
        if (afficheur_btn.size() == 0)
        {
            scroller->hide();
            scroller->redraw();
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
