/*
General purpose store for storing common data
Thread safe

This could be splitted into multiple files to improve readability
*/
#include "store.hpp"
#include "type.hpp"
#include <FL/Fl_Widget.H>
#include <cstring>
#include <mutex>
#include <list>
#include <shared_mutex>
#include "sqlite/request_manager.hpp"
#include "sqlite/afficheur_data.hpp"
#include "sqlite/trip_data.hpp"
#include <ctime>
#define CLOCK_TEXT_LENGTH 7

Store::Store() : gps_state(new GpsState(0, 0, GPSStatus::STARTING)),
                 line_active(false),
                 current_trip(nullptr),
                 next_stop(std::string("HORS SERVICE")),
                 next_stop_time(std::string("/")),
                 next_stop_overwrite(0),
                 next_stop_idx(0),
                 current_line(std::string("/")),
                 dir(std::string("R")),
                 region(std::string("045")),
                 zone(std::string("01")),
                 voyage(std::string("0028")),
                 odm(std::string("143066")),
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
    original_girouette = new AfficheurData(id, text, line); // Keep two different pointers to avoid freeing the original girouette
    afficheurs->push_back(current_girouette);

    request_manager = new RequstManager();
    request_manager->open();
}

void Store::pop_afficheur_id(size_t idx)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (idx >= afficheurs->size())
        return;

    if (afficheurs->size() == 1)
        return;

    auto it = afficheurs->begin();
    std::advance(it, idx);
    delete *it;
    afficheurs->erase(it);

    current_girouette = *afficheurs->begin();
    refresh_gui();
}

void Store::stop_service()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    line_active = false;
    delete current_trip;
    current_trip = nullptr;

    std::string id("01");
    std::string text("HORS SERVICE");
    std::string line("");
    next_stop_idx = 0;
    for (auto it = afficheurs->begin(); it != afficheurs->end(); ++it)
        delete *it;
    afficheurs->clear();

    current_girouette = new AfficheurData(id, text, line);
    original_girouette = new AfficheurData(id, text, line);
    afficheurs->push_back(current_girouette);
    refresh_gui();
}

void Store::reset_girouette()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    for (auto it = afficheurs->begin(); it != afficheurs->end(); ++it)
        delete *it;
    afficheurs->clear();

    current_girouette = new AfficheurData(original_girouette);
    afficheurs->push_back(current_girouette);
    refresh_gui();
}

void Store::set_original_girouette(AfficheurData *girouette)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    original_girouette = new AfficheurData(girouette);
    refresh_gui();
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
    if (has_active && active_widget != nullptr)
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
    for (auto it = afficheurs->begin(); it != afficheurs->end(); ++it)
        delete *it;
    afficheurs->clear();

    AfficheurData *new_afficheur = new AfficheurData(girouette);
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

//  The pointer should not be freed
GpsState Store::get_gps_state() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return GpsState(gps_state);
}

std::string Store::get_current_line() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return current_line.c_str();
}

std::string Store::get_dir() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return dir.c_str();
}

std::string Store::get_region() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return region.c_str();
}

std::string Store::get_zone() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return zone.c_str();
}

std::string Store::get_voyage() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return voyage.c_str();
}

std::string Store::get_odm() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return odm.c_str();
}

AfficheurData Store::get_current_girouette() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return AfficheurData(current_girouette);
}

size_t Store::get_girouettes_size() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return afficheurs->size();
}

std::list<AfficheurData *> *Store::get_girouettes() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    std::list<AfficheurData *> *afficheurs = new std::list<AfficheurData *>();
    for (auto it = this->afficheurs->begin(); it != this->afficheurs->end(); ++it)
        afficheurs->push_back(new AfficheurData(*it));

    return afficheurs;
}

TripData Store::get_current_trip() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return TripData(current_trip);
}

std::string Store::get_next_stop() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return next_stop.c_str();
}

std::string Store::get_next_stop_time() const
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
    free((void *)current_trip);
    current_trip = trip;
    line_active = true;
    next_stop_idx = 0;
    refresh_gui();
}

std::string Store::get_delay() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return delay;
}

void Store::stop_overwrite()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (!line_active)
        return;
    next_stop_overwrite = false;
    next_stop_idx = current_trip->get_theorical_stop()->idx;
    refresh_stop();
}

void Store::refresh_stop()
{
    const StopTime *stop = current_trip->get_nth_stop(next_stop_idx);
    refresh_delay_unsecure();
    next_stop = stop->stop_name;
    next_stop_time = get_arrival_time(stop);
}

void Store::go_to_next_stop()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (!line_active)
        return;

    size_t max_size = current_trip->get_stop_times()->size() - 1;
    if (next_stop_idx > max_size)
        return;

    next_stop_overwrite = true;
    next_stop_idx++;
    refresh_stop();
    refresh_gui();
}

void Store::go_to_prev_stop()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (!line_active)
        return;

    if (next_stop_idx == 0)
        return;

    next_stop_overwrite = true;
    next_stop_idx--;
    refresh_stop();
    refresh_gui();
}

void Store::set_stop_index(size_t idx, bool refresh)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    size_t max_size = current_trip->get_stop_times()->size() - 1;
    if (idx > max_size)
        idx = max_size;
    next_stop_idx = idx;
    if (refresh)
        refresh_stop();
    refresh_gui();
}

void Store::refresh_delay_unsecure()
{
    if (!line_active && !next_stop_overwrite)
        return;

    std::time_t rawtime = std::time(nullptr);
    struct tm *timeinfo = std::localtime(&rawtime);
    int current_time = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;
    int scheduled_time = current_trip->get_nth_stop(next_stop_idx)->arrival_time;

    char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
    if (!buffer)
        throw std::bad_alloc();

    time_t total_delay = current_time - scheduled_time;

    if (total_delay < 0)
    {
        total_delay = -total_delay;
        struct tm *timeinfo = std::gmtime(&total_delay);
        std::strftime(buffer, CLOCK_TEXT_LENGTH, "-%H:%M", timeinfo);
    }
    else
    {
        struct tm *timeinfo = std::gmtime(&total_delay);
        std::strftime(buffer, CLOCK_TEXT_LENGTH, "+%H:%M", timeinfo);
    }
    delay = std::string(buffer);

    free(buffer);

    refresh_gui();
}

void Store::refresh_delay()
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    refresh_delay_unsecure();
}

bool Store::is_next_stop_overwrite() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return next_stop_overwrite;
}

size_t Store::get_next_stop_idx() const
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    return next_stop_idx;
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
    delete request_manager;
    delete gps_state;
}
