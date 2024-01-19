#include "../layout.hpp"

#include "custom_tabs.hpp"
#include "custom_group.hpp"
#include "../menu/billetique.hpp"
#include "../menu/afficheurs.hpp"
#include "../menu/ordibus.hpp"
#include "../menu/pilotauto.hpp"
#include "../menu/maintenance.hpp"

#include "../store.hpp"

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>

const int TABS_AMMOUNT = 5;
const char *TABS_LABELS[TABS_AMMOUNT] = {
    "Afficheurs",
    "Billettique",
    "Ordibus",
    "Pilot. Auto.",
    "Maintenance"};

static void change_tab_cb(Fl_Widget *w, void *data)
{
    size_t *tab = (size_t *)data;
    TECTabs *tabs = (TECTabs *)w->parent();
    tabs->change_tab(*tab);
}

void TECTabs::change_tab(int tab)
{
    if (tab == active_tab)
        return;

    store->set_active_widget(active_box[tab]);
    active_box[active_tab]->hide();
    active_tab = tab;
    active_box[active_tab]->show();
    redraw();
}

TECTabs::TECTabs(int x, int y, int w, int h, Store *store, const char *l)
    : TECGroup(x, y, w, h, l), active_tab(2), store(store)
{
    active_box = (Fl_Box **)malloc(sizeof(Fl_Box) * TABS_AMMOUNT);
    if (!active_box)
        throw std::bad_alloc();
    begin();
    int offset = 0;
    for (int i = 0; i < TABS_AMMOUNT; ++i)
    {
        int dx, dy, W, H;
        fl_font(FL_BOLD, 14);
        fl_text_extents(TABS_LABELS[i], dx, dy, W, H);
        W += 20;
        Fl_Button *btn = new Fl_Button(offset, y, W, TABS_HEIGHT);
        offset += W;

        char *text = (char *)TABS_LABELS[i];
        btn->label(text);
        btn->labelcolor(FL_WHITE);
        btn->labelsize(14);
        btn->labelfont(FL_BOLD);
        size_t *data = new size_t;
        *data = i;
        btn->user_data(data);
        if (i == 1 || i == 4)
            btn->deactivate();
        btn->callback(change_tab_cb, data); // pass twice the address to free later
    }
    end();

    // Write here to prevent layer to be on top of tabs
    Afficheurs *afficheurs = new Afficheurs(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT, store);
    afficheurs->hide();
    active_box[0] = (Fl_Box *)afficheurs;

    Billetique *billetique = new Billetique(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT, store);
    billetique->hide();
    active_box[1] = (Fl_Box *)billetique;

    Ordibus *ordibus = new Ordibus(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT, store);
    ordibus->hide();
    active_box[2] = (Fl_Box *)ordibus;

    Pilotauto *pilotauto = new Pilotauto(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT, store);
    pilotauto->hide();
    active_box[3] = (Fl_Box *)pilotauto;

    Maintenance *maintenance = new Maintenance(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    maintenance->hide();
    active_box[4] = (Fl_Box *)maintenance;

    store->set_active_widget(active_box[active_tab]);
    active_box[active_tab]->show();
}

void TECTabs::draw()
{
    for (int i = 0; i < children(); ++i)
    {
        Fl_Widget *w = child(i);
        if (i == active_tab)
        {
            w->box(FL_BORDER_BOX);
            w->color(SECONDARY);
        }
        else
        {

            w->color(PRIMARY);
            w->box(FL_FLAT_BOX);
        }
    }

    Fl_Group::draw();
}

TECTabs::~TECTabs()
{
    for (int i = 0; i < TABS_AMMOUNT; ++i)
    {
        if (active_box[i] != nullptr)
        {
            delete active_box[i];
            active_box[i] = nullptr;
        }
    }
    free(active_box);
}