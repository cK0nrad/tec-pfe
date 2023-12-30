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

Store::Store() : active_widget(nullptr), has_active(false), line_indicator(nullptr), line_indicator_active(false)
{
    afficheurs = new std::list<AfficheurData *>();
    char *id = (char *)malloc(sizeof(char) * 3);
    char *text = (char *)malloc(sizeof(char) * 13);
    char *line = (char *)malloc(sizeof(char) * 1);
    strcpy(id, "01");
    strcpy(text, "HORS SERVICE");
    strcpy(line, "");
    current_girouette = new AfficheurData(id, text, line);
    afficheurs->push_back(current_girouette);

    request_manager = new RequstManager();
    request_manager->open();

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

// RM has it's own thread safety
RequstManager *Store::get_request_manager()
{
    return request_manager;
}

Store::~Store()
{
    for (auto it = afficheurs->begin(); it != afficheurs->end(); it++)
        delete *it;

    delete afficheurs;
    delete current_girouette;

    free(current_line);
    free(dir);
    free(region);
    free(zone);
    free(voyage);
    free(odm);
    delete request_manager;
    delete gps_state;
}
