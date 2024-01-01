#ifndef STORE
#define STORE

#include "type.hpp"
#include <shared_mutex>
#include <list>
#include <FL/Fl_Widget.H>

class RequstManager;
class AfficheurData;
class TripData;

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

    void set_current_line(std::string line);
    void set_dir(std::string dir);
    void set_region(std::string region);
    void set_zone(std::string zone);
    void set_voyage(std::string voyage);
    void set_odm(std::string odm);

    void push_girouette(AfficheurData *girouette);
    void replace_girouette(AfficheurData *girouette);
    void pop_afficheur_id(size_t idx);
    void change_girouette(size_t idx);
    void set_next_stop(std::string stop, std::string time);

    void set_trip(TripData *trip);
    void set_delay(std::string delay);

    void set_original_girouette(AfficheurData *girouette);
    void reset_girouette();

    void stop_service();
    void stop_overwrite();
    void go_to_next_stop();
    void go_to_prev_stop();

    void set_stop_index(size_t idx);
    void refresh_delay();

    // Getters (no pointer to avoid cross access over threads)
    // less efficient but safer
    GpsState get_gps_state() const;
    bool is_line_active() const;

    TripData get_current_trip() const;

    std::string get_current_line() const;
    std::string get_dir() const;
    std::string get_region() const;
    std::string get_zone() const;
    std::string get_voyage() const;
    std::string get_odm() const;

    std::string get_next_stop() const;
    std::string get_next_stop_time() const;

    AfficheurData get_current_girouette() const;
    std::list<AfficheurData *> *get_girouettes() const;
    size_t get_girouettes_size() const;
    std::string get_delay() const;

    // independant thread safety
    RequstManager *get_request_manager();
    bool is_next_stop_overwrite() const;
    size_t get_next_stop_idx() const;



private:
    void refresh_delay_unsecure();
    void refresh_gui();
    void refresh_stop();

    RequstManager *request_manager;
    mutable std::shared_mutex mutex;
    GpsState *gps_state;

    bool line_active;
    TripData *current_trip;

    std::string delay;

    std::string next_stop;
    std::string next_stop_time;
    bool next_stop_overwrite;
    size_t next_stop_idx;


    std::string current_line;
    std::string dir;
    std::string region;
    std::string zone;
    std::string voyage; // trip_id
    std::string odm;    // ordre de marche ? unknown for now

    std::list<AfficheurData *> *afficheurs;
    AfficheurData *current_girouette;
    AfficheurData *original_girouette;

    // tiny hack to refresh the gui has the store is updated
    // idk if there is a better way to do this (wraping into
    // a widget linked to the store each sub-module?)
    Fl_Widget *active_widget;
    bool has_active;
    Fl_Widget *line_indicator;
    bool line_indicator_active;
};

#endif