/*
General purpose store for storing common data
Thread safe
*/

#include "type.hpp"
#include "store.hpp"
#include <FL/Fl_Widget.H>

#include <mutex>
#include <shared_mutex>
Store::Store()
{
    gps_state = new GpsState(0, 0, GPSStatus::STARTING);
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

const bool Store::is_line_active() const
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
    if (active_widget)
        active_widget->redraw();
}
Store::~Store()
{
    delete gps_state;
}
