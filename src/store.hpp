#ifndef STORE
#define STORE

#include "type.hpp"
#include <shared_mutex>
#include <FL/Fl_Widget.H>
#include "sqlite/request_manager.hpp"

class Store
{

public:
    Store();
    ~Store();
    // Setters
    void set_gps_position(double lat, double long_);
    void set_active_widget(Fl_Widget *widget);
    void set_line_indicator(Fl_Widget *widget);
    void set_gps_status(GPSStatus status);
    void set_line_active(bool active);

    void set_current_line(const char *line);
    void set_dir(const char *dir);
    void set_region(const char *region);
    void set_zone(const char *zone);
    void set_voyage(const char *voyage);
    void set_odm(const char *odm);

    // Getters
    const GpsState *get_gps_state() const;
    bool is_line_active() const;

    const char *get_current_line() const;
    const char *get_dir() const;
    const char *get_region() const;
    const char *get_zone() const;
    const char *get_voyage() const;
    const char *get_odm() const;

    // independant thread safety
    RequstManager *get_request_manager();

private:
    void refresh_gui();
    RequstManager *request_manager;
    mutable std::shared_mutex mutex;
    GpsState *gps_state;
    bool line_active = false;
    char *current_line = nullptr; // route_id
    char *dir = nullptr;
    char *region = nullptr;
    char *zone = nullptr;
    char *voyage = nullptr; // trip_id
    char *odm = nullptr;    // ordre de marche ? unknown for now

    // tiny hack to refresh the gui has the store is updated
    // idk if there is a better way to do this (wraping into
    // a widget linked to the store each sub-module?)
    Fl_Widget *active_widget = nullptr;
    Fl_Widget *line_indicator = nullptr;
};

#endif