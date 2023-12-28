// Include our FLTK headers
#include <FL/Fl.H>
#include <FL/Fl_Window.H>

// Include our layout header
#include "layout.hpp"

// Get our custom elements
#include "gui/custom_tabs.hpp"
#include "gui/custom_group.hpp"
#include "gui/time.hpp"

#include "gps/gps.hpp"

int main()
{

    Store *store = new Store();
    if(!store){
        return 1;
    }

    // // Start GPS tshread
    GPS *gps_receiver = new GPS(store);
    gps_receiver->start();

    Fl_Window *window = new Fl_Window(WIDTH, HEIGHT);
    // window->border(false);
    window->color(FL_BLACK);

    TECClock *clock = new TECClock(0, 0, WIDTH, TABS_HEIGHT, store);
    clock->color(PRIMARY);
    store->set_line_indicator(clock);

    TECTabs *custom_tabs = new TECTabs(0, TABS_HEIGHT, WIDTH, TABS_HEIGHT, store);
    custom_tabs->color(FL_WHITE);
    custom_tabs->end();

    window->end();
    window->show();

    int err_code = Fl::run();

    // Stop GPS thread
    gps_receiver->stop();
    gps_receiver->await();

    delete custom_tabs;
    delete clock;
    delete window;
    delete store;
    return err_code;
}