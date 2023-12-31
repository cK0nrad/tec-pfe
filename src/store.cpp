/*
General purpose store for storing common data
Thread safe

This could be splitted into multiple files to improve readability
*/

#include "type.hpp"
#include "store.hpp"
#include <FL/Fl_Widget.H>
#include <cstring>
#include <mutex>
#include <list>
#include <shared_mutex>
#include "sqlite/request_manager.hpp"

Store::Store() : gps_state(new GpsState(0, 0, GPSStatus::STARTING)),
                 line_active(false),
                 next_stop(std::string("HORS SERVICE")),
                 next_stop_time(std::string("/")),
                 current_line(std::string("3820213123123")),
                 dir(std::string("R")),
                 region(std::string("045")),
                 zone(std::string("01")),
                 voyage(std::string("0028")),
                 odm(std::string("0412")),
                 active_widget(nullptr),
                 has_active(false),
                 line_indicator(nullptr),
                 line_indicator_active(false)
{
    std::string id("01");
    std::string text("HORS SERVICE");
    std::string line("");

    afficheurs = new std::list<AfficheurData *>();
    current_girouette = new AfficheurData(id, text, line);
    afficheurs->push_back(current_girouette);

    request_manager = new RequstManager();
    request_manager->open();
    //
    // set_trip(request_manager->get_trip("6191"));
    set_trip(request_manager->get_trip("6229"));
}

void Store::set_next_stop(std::string stop, std::string time)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    next_stop = stop;
    next_stop_time = time;
    refresh_gui();
}

void Store::set_gps_position(double lat, double long_)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    gps_state->set_lat(lat);
    gps_state->set_lon(long_);
    refresh_gui();
}

void Store::set_gps_status(GPSStatus status)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    gps_state->set_status(status);
    refresh_gui();
}

void Store::set_active_widget(Fl_Widget *widget)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    active_widget = widget;
    has_active = true;
    // refresh_gui() <- not needed as the widget will be redrawn when we change
}

void Store::set_line_active(bool active)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    line_active = active;
    if (line_indicator_active)
        line_indicator->redraw();
}

void Store::set_line_indicator(Fl_Widget *widget)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    line_indicator = widget;
    line_indicator_active = true;
}

void Store::refresh_gui()
{
    if (has_active)
        active_widget->redraw();
}

void Store::set_current_line(std::string current_line)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->current_line = current_line;
    refresh_gui();
}

void Store::set_dir(std::string dir)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->dir = dir;
    refresh_gui();
}

void Store::set_region(std::string region)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->region = region;
    refresh_gui();
}

void Store::set_zone(std::string zone)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->zone = zone;
    refresh_gui();
}

void Store::set_voyage(std::string voyage)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->voyage = voyage;
    refresh_gui();
}

void Store::set_odm(std::string odm)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->odm = odm;
    refresh_gui();
}

void Store::push_girouette(AfficheurData *girouette)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    AfficheurData *new_afficheur = new AfficheurData(girouette);
    afficheurs->push_back(new_afficheur);
    refresh_gui();
}

void Store::replace_girouette(AfficheurData *girouette)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    AfficheurData *new_afficheur = new AfficheurData(girouette);

    for (auto it = afficheurs->begin(); it != afficheurs->end(); ++it)
        delete *it;
    afficheurs->clear();

    current_girouette = new_afficheur;
    afficheurs->push_back(new_afficheur);
    refresh_gui();
}

void Store::change_girouette(size_t idx)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    current_girouette = *std::next(afficheurs->begin(), idx);
    refresh_gui();
}

/// GETTERS
bool Store::is_line_active() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return line_active;
}

// The pointer should not be freed
const GpsState *Store::get_gps_state() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return gps_state;
}

const char *Store::get_current_line() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return current_line.c_str();
}

const char *Store::get_dir() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return dir.c_str();
}

const char *Store::get_region() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return region.c_str();
}

const char *Store::get_zone() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return zone.c_str();
}

const char *Store::get_voyage() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return voyage.c_str();
}

const char *Store::get_odm() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return odm.c_str();
}

const AfficheurData *Store::get_current_girouette() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return current_girouette;
}

const std::list<AfficheurData *> *Store::get_girouettes() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return afficheurs;
}

const TripData *Store::get_current_trip() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return current_trip;
}

const char *Store::get_next_stop() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return next_stop.c_str();
}

const char *Store::get_next_stop_time() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return next_stop_time.c_str();
}

void Store::set_delay(std::string delay)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->delay = delay;
    refresh_gui();
}

void Store::set_trip(TripData *trip)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    current_trip = trip;
    line_active = true;
    refresh_gui();
}

std::string Store::get_delay() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return delay;
}


// RM has it's own thread safety
RequstManager *Store::get_request_manager()
{
    return request_manager;
}

Store::~Store()
{
    for (auto it = afficheurs->begin(); it != afficheurs->end(); it++)
        delete *it;

    if (line_active)
        delete current_trip;

    delete afficheurs;
    delete current_girouette;
    delete request_manager;
    delete gps_state;
}
