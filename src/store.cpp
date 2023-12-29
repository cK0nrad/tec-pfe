/*
General purpose store for storing common data
Thread safe
*/

#include "type.hpp"
#include "store.hpp"
#include <FL/Fl_Widget.H>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include "sqlite/request_manager.hpp"

Store::Store(): active_widget(nullptr), line_indicator(nullptr)
{
    request_manager = new RequstManager();
    request_manager->open();
    std::list<AfficheurData *> afficheurs = request_manager->get_afficheur("1");
    for (auto const &afficheur : afficheurs)
    {
        printf("%s\n", afficheur->get_id());
        delete afficheur;
    }

    gps_state = new GpsState(0, 0, GPSStatus::STARTING);

    current_line = (char *)malloc(sizeof(char) * 14);
    strcpy(current_line, "3820213123123");
    dir = (char *)malloc(sizeof(char) * 2);
    strcpy(dir, "R");

    region = (char *)malloc(sizeof(char) * 4);
    strcpy(region, "045");

    zone = (char *)malloc(sizeof(char) * 3);
    strcpy(zone, "01");

    voyage = (char *)malloc(sizeof(char) * 5);
    strcpy(voyage, "0028");

    odm = (char *)malloc(sizeof(char) * 5);
    strcpy(odm, "0412");
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
    // refresh_gui() <- not needed as the widget will be redrawn when we change
}

void Store::set_line_active(bool active)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    line_active = active;
    if (line_indicator)
        line_indicator->redraw();
}

void Store::set_line_indicator(Fl_Widget *widget)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    line_indicator = widget;
}

void Store::refresh_gui()
{
    if (active_widget != nullptr)
        active_widget->redraw();
}

void Store::set_current_line(const char *line)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    free(current_line);
    current_line = (char *)line;
    refresh_gui();
}

void Store::set_dir(const char *dir)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    free(this->dir);
    this->dir = (char *)dir;
    refresh_gui();
}

void Store::set_region(const char *region)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    free(this->region);
    this->region = (char *)region;
    refresh_gui();
}

void Store::set_zone(const char *zone)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    this->zone = (char *)zone;
    refresh_gui();
}

void Store::set_voyage(const char *voyage)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    free(this->voyage);
    this->voyage = (char *)voyage;
    refresh_gui();
}

void Store::set_odm(const char *odm)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    free(this->odm);
    this->odm = (char *)odm;
    refresh_gui();
}

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
    return current_line;
}

const char *Store::get_dir() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return dir;
}

const char *Store::get_region() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return region;
}

const char *Store::get_zone() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return zone;
}

const char *Store::get_voyage() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return voyage;
}

const char *Store::get_odm() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return odm;
}

//RM has it's own thread safety
RequstManager *Store::get_request_manager()
{
    return request_manager;
}

Store::~Store()
{
    free(current_line);
    free(dir);
    free(region);
    free(zone);
    free(voyage);
    free(odm);
    delete request_manager;
    delete gps_state;
}
