#ifndef STORE
#define STORE

#include "type.hpp"
#include <shared_mutex>
#include <FL/Fl_Widget.H>
class Store
{

public:
    Store();
    ~Store();
    // Setters
    void set_gps_position(double lat, double long_);
    void set_active_widget(Fl_Widget *widget);
    void set_gps_status(GPSStatus status);
    void set_line_active(bool active);
    void set_line_indicator(Fl_Widget *widget);

    // Getters
    const GpsState *get_gps_state() const;
    const bool is_line_active() const;

private:
    void refresh_gui();
    mutable std::shared_mutex mutex;
    GpsState *gps_state;
    bool line_active = false;

    // tiny hack to refresh the gui has the store is updated
    // idk if there is a better way to do this (wraping into
    // a widget linked to the store each sub-module?)
    Fl_Widget *active_widget;
    Fl_Widget *line_indicator;
};

#endif