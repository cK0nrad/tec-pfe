#include <string>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_BMP_Image.H>

#include "pilotauto.hpp"
#include "../layout.hpp"
#include "../store.hpp"
#include "../type.hpp"
#include "../sqlite/trip_data.hpp"
#include "../sqlite/afficheur_data.hpp"

#define MARGIN 10

const char *started = "LOCALISE";
const char *errored = "ERREUR GPS";
const char *starting = "DEMARAGE";
const char *unknown = "UNKNOWN";

static void next_cb(Fl_Widget *, void *data)
{
    Store *store = (Store *)data;
    store->go_to_next_stop();
}

static void prev_cb(Fl_Widget *, void *data)
{
    Store *store = (Store *)data;
    store->go_to_prev_stop();
}

static void overwrite_cb(Fl_Widget *, void *data)
{
    Store *store = (Store *)data;
    store->stop_overwrite();
}

Pilotauto::~Pilotauto()
{
    Fl::delete_widget(overwrite);
    Fl::delete_widget(next);
    Fl::delete_widget(prev);
    delete next_img;
    delete prev_img;
}

Pilotauto::Pilotauto(int x, int y, int w, int h, Store *store, const char *l)
    : Fl_Group(x, y, w, h, l), store(store)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    size_t bottom = h + y;

    overwrite = new Fl_Button(15 + 560 + 15 + 25, bottom - 3 * TABS_HEIGHT - 75, 2 * 75 + 20, 75, "REPRISE THEORIQUE");
    overwrite->labelsize(16);
    overwrite->callback(overwrite_cb, store);
    overwrite->deactivate();

    next_img = new Fl_BMP_Image("./src/assets/up.bmp");
    next = new Fl_Button(15 + 560 + 15 + 25, bottom - 3 * TABS_HEIGHT + 15, 75, 75);
    next->image(next_img);
    next->callback(next_cb, store);
    next->deactivate();

    prev_img = new Fl_BMP_Image("./src/assets/down.bmp");
    prev = new Fl_Button(15 + 560 + 15 + 75 + 20 + 25, bottom - 3 * TABS_HEIGHT + 15, 75, 75);
    prev->image(prev_img);
    prev->callback(prev_cb, store);
    prev->deactivate();

    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();
    end();
}

void Pilotauto::draw()
{
    GPSStatus status = store->get_gps_state().get_status();
    const char *status_text = nullptr;
    switch (status)
    {
    case GPSStatus::RUNNING:
        status_text = started;
        break;
    case GPSStatus::STARTING:
        status_text = starting;
        break;
    case GPSStatus::ERROR:
        status_text = errored;
        break;
    default:
        status_text = unknown;
        break;
    }

    if (status == 2)
    {
        overwrite->activate();
        next->activate();
        prev->activate();
    }
    else
    {
        overwrite->deactivate();
        next->deactivate();
        prev->deactivate();
    }

    Fl_Group::draw();

    size_t current_x = 15;
    size_t bottom = h() + y();
    // odm
    fl_color(SECONDARY_LIGHT);
    fl_rectf(current_x, 3 * TABS_HEIGHT + MARGIN, 100, TABS_HEIGHT);
    current_x += 100 + MARGIN;

    // etat
    fl_rectf(current_x, 3 * TABS_HEIGHT + MARGIN, 350, TABS_HEIGHT);
    current_x += 350 + MARGIN;

    // retard
    fl_rectf(current_x, 3 * TABS_HEIGHT + MARGIN, 100, TABS_HEIGHT);

    // aff exterieur
    fl_rectf(15, 5 * TABS_HEIGHT + MARGIN, 570, TABS_HEIGHT);

    // Arret suivant
    fl_rectf(15, bottom - 2 * TABS_HEIGHT + MARGIN, 75, TABS_HEIGHT);
    fl_rectf(15 + 75 + MARGIN, bottom - 2 * TABS_HEIGHT + MARGIN, 480, TABS_HEIGHT);

    current_x = 15;
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 20);

    fl_draw("ODM", 15, 3 * TABS_HEIGHT);
    fl_draw(store->get_odm().c_str(), 15, 3 * TABS_HEIGHT + MARGIN, 100, TABS_HEIGHT, FL_ALIGN_CENTER);
    current_x += 100 + MARGIN;

    fl_draw("Etat", current_x, 3 * TABS_HEIGHT);

    fl_draw(status_text, current_x, 3 * TABS_HEIGHT + MARGIN, 350, TABS_HEIGHT, FL_ALIGN_CENTER);
    current_x += 350 + MARGIN;

    fl_draw("Retard", current_x, 3 * TABS_HEIGHT);

    if (store->is_line_active())
    {
        fl_draw(store->get_delay().c_str(), current_x, 3 * TABS_HEIGHT + MARGIN, 100, TABS_HEIGHT, FL_ALIGN_CENTER);
    }

    fl_draw("Afficheurs extérieurs", 15, 5 * TABS_HEIGHT);

    const char *formatted = store->get_current_girouette().get_formatter();
    fl_draw(formatted, 15, 5 * TABS_HEIGHT + MARGIN, 560, TABS_HEIGHT, FL_ALIGN_CENTER);
    free((void *)formatted);

    fl_draw("Arret suivant", 15, bottom - 2 * TABS_HEIGHT);
    if (store->is_line_active())
    {
        fl_draw(store->get_next_stop().c_str(), 15 + 75 + MARGIN, bottom - 2 * TABS_HEIGHT + MARGIN, 480, TABS_HEIGHT, FL_ALIGN_CENTER);
        fl_draw(store->get_next_stop_time().c_str(), 15, bottom - 2 * TABS_HEIGHT + MARGIN, 75, TABS_HEIGHT, FL_ALIGN_CENTER);
    }
}