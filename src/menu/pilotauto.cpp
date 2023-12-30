#include <string>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>

#include "pilotauto.hpp"
#include "../layout.hpp"
#include "../store.hpp"
#include "../type.hpp"

#define MARGIN 10

const char *started = "LOCALISE";
const char *errored = "ERREUR GPS";
const char *starting = "DEMARAGE";
const char *unknown = "UNKNOWN";

struct LineState_t
{
    char *line;
    char *odm;
    int current_stop;
} typedef LineState;

Pilotauto::Pilotauto(int x, int y, int w, int h, Store *store, const char *l)
    : Fl_Group(x, y, w, h, l), store(store)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    LineState *line = (LineState *)malloc(sizeof(LineState));
    line->line = (char *)"2";
    line->odm = (char *)"143066";
    line->current_stop = 0;

    user_data((void *)line);

    end();
}

void Pilotauto::draw()
{
    Fl_Group::draw();
    LineState *line = (LineState *)user_data();
    if (!line)
        return;

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
    fl_draw(line->odm, 15, 3 * TABS_HEIGHT + MARGIN, 100, TABS_HEIGHT, FL_ALIGN_CENTER);
    current_x += 100 + MARGIN;

    fl_draw("Etat", current_x, 3 * TABS_HEIGHT);

    GPSStatus status = store->get_gps_state()->get_status();
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
    fl_draw(status_text, current_x, 3 * TABS_HEIGHT + MARGIN, 350, TABS_HEIGHT, FL_ALIGN_CENTER);
    current_x += 350 + MARGIN;

    fl_draw("Retard", current_x, 3 * TABS_HEIGHT);
    fl_draw("Afficheurs extérieurs", 15, 5 * TABS_HEIGHT);

    const char *formatted = store->get_current_girouette()->get_formatter();
    fl_draw(formatted, 15, 5 * TABS_HEIGHT + MARGIN, 560, TABS_HEIGHT, FL_ALIGN_CENTER);
    free((void *)formatted);

    fl_draw("Arret suivant", 15, bottom - 2 * TABS_HEIGHT);
}