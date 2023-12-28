#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_draw.H>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "time.hpp"
#include "../store.hpp"

// 00:00:0000 00:00:00 19 + \0
#define CLOCK_TEXT_LENGTH 20

const char *service_fermee = "SERVICE FERME";
const char *service_normal = "SERVICE NORRMAL";

static void Timer_CB(void *v)
{
    ((TECClock *)v)->update();
    Fl::repeat_timeout(1.0, Timer_CB, v);
}

TECClock::TECClock(int X, int Y, int W, int H, Store *store, const char *L)
    : Fl_Box(X, Y, W, H, L), store(store)
{
    box(FL_FLAT_BOX);

    std::time_t rawtime = std::time(nullptr);
    struct tm *timeinfo = std::localtime(&rawtime);
    char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
    std::strftime(buffer, CLOCK_TEXT_LENGTH, "%d/%m/%y %H:%M:%S", timeinfo);
    user_data(buffer);
    Fl::add_timeout(1.0, Timer_CB, (void *)this);
}

void TECClock::update()
{
    std::time_t rawtime = std::time(nullptr);
    struct tm *timeinfo = std::localtime(&rawtime);

    //Could just reuse the buffer but I don't know if it's safe for fltk
    if (user_data() != nullptr)
        free((void *)user_data());

    char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
    std::strftime(buffer, CLOCK_TEXT_LENGTH, "%d/%m/%y %H:%M:%S", timeinfo);
    user_data(buffer);
    redraw();
}
void TECClock::draw()
{
    Fl_Box::draw();
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 15);

    bool line_active = store->is_line_active();
    if (line_active)
    {
        fl_color(FL_GREEN);
        fl_draw(service_normal, 0, 0, 150, h(), FL_ALIGN_CENTER);
    }
    else
    {
        fl_draw(service_fermee, 0, 0, 150, h(), FL_ALIGN_CENTER);
    }

    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 20);
    fl_draw((char *)user_data(), 175, 0, 150, h(), FL_ALIGN_CENTER);
}

TECClock::~TECClock()
{
    if (user_data() != nullptr)
        free((void *)user_data());
    Fl::remove_timeout(Timer_CB, (void *)this);
}