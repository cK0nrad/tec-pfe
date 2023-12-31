#ifndef TRIP_DATA
#define TRIP_DATA

#include <vector>
#include <string>
#include <math.h>

double earth_distance(double lat1, double lon1, double lat2, double lon2);
double sqr_distance(double lat1, double lon1, double lat2, double lon2);

struct StopTime;
struct Point;


// The class is duplicated whenever it's needed so no thread safety is needed

class TripData
{

public:
    TripData(
        std::string trip_id,
        std::string route_id,
        std::string afficheur_id,
        std::string route_short_name,
        std::string route_long_name,
        std::vector<Point *> *shape,
        std::vector<StopTime *> *stops_times);
    TripData(const TripData *trip);
    ~TripData();
    std::string get_trip_id() const;
    std::string get_afficheur_id() const;

    const std::vector<Point *> *get_trip() const;
    const std::vector<StopTime *> *get_stop_times() const;

    const StopTime *get_next_stop(double lat, double long_) const;
    const StopTime *get_theorical_stop() const;
    const StopTime *get_nth_stop(size_t idx) const;

    const std::vector<size_t> *get_cache_nearest_stop() const;
    const std::vector<size_t> *get_cache_nearest_shape() const;
    const std::vector<double> *get_cache_stop_distances() const;
    const std::vector<Point *> *get_shape() const;
    
private:
    std::string trip_id;
    std::string route_id;
    std::string afficheur_id;
    std::string route_short_name;
    std::string route_long_name;
    std::vector<Point *> *shape;
    std::vector<StopTime *> *stop_times;
    std::vector<size_t> cache_nearest_stop;
    std::vector<size_t> cache_nearest_shape;
    std::vector<double> cache_stop_distances;
};

#endif