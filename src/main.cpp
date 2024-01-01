// Include our FLTK headers
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>

// Include our layout header
#include "layout.hpp"

// Get our custom elements
#include "sqlite/request_manager.hpp"

#include "gui/custom_tabs.hpp"
#include "gui/custom_group.hpp"
#include "gui/time.hpp"

#include "gps/gps.hpp"
#include "girouette/girouette.hpp"
#include "data_sync/sync.hpp"

#include "store.hpp"


int main()
{
    Store *store = new Store();

    // Start GPS tshread
    GPS *gps_receiver = new GPS(store);
    gps_receiver->start();

    // Start Girouett thread
    Girouette *girouette = new Girouette(store);
    girouette->start();

    // Start Girouett thread
    SyncClient *data_sync = new SyncClient(store);
    data_sync->start();

    Fl_Double_Window *window = new Fl_Double_Window(WIDTH, HEIGHT);
    window->color(FL_BLACK);
    window->default_cursor(FL_CURSOR_NONE);
    window->cursor(FL_CURSOR_NONE);

    TECClock *clock = new TECClock(0, 0, WIDTH, TABS_HEIGHT, store);
    clock->color(PRIMARY);
    store->set_line_indicator(clock);

    TECTabs *custom_tabs = new TECTabs(0, TABS_HEIGHT, WIDTH, TABS_HEIGHT, store);
    custom_tabs->color(FL_WHITE);
    custom_tabs->end();

    window->end();
    window->show();
    int err_code = Fl::run();

    // Stop GPS & girouette thread
    delete data_sync;
    delete gps_receiver;
    delete girouette;

    delete window;

    delete store;
    return err_code;
}