#ifndef TRIP_DATA
#define TRIP_DATA

#include <vector>
#include <string>
#include <math.h>


static double earth_distance(double lat1, double lon1, double lat2, double lon2)
{
    static const double R = 6378.137;

    double phi1 = lat1 * M_PI / 180;
    double phi2 = lat2 * M_PI / 180;
    double dLat = phi2 - phi1;
    double dLon = (lon2 - lon1) * M_PI / 180;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(phi1) * cos(phi2) *
                   sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    double d = R * c;
    return d;
}

static double sqr_distance(double lat1, double lon1, double lat2, double lon2)
{
    double dLat = (lat2 - lat1);
    double dLon = (lon2 - lon1);
    return dLat * dLat + dLon * dLon;
}


typedef struct Point_t
{
    double lat;
    double long_;
} Point;

typedef struct StopTime_t
{
    std::string stop_id;
    std::string stop_name;
    StopTime_t *next;
    Point position;
    bool is_end;
    int arrival_time;
    size_t idx;
} StopTime;

class TripData
{

public:
    TripData(
        std::string trip_id,
        std::string route_id,
        std::string route_short_name,
        std::string route_long_name,
        std::vector<Point *> *shape,
        std::vector<StopTime *> *stops_times);
    ~TripData();
    const std::vector<Point *> *get_trip() const;
    const std::string &get_trip_id() const;
    const std::vector<StopTime *> *get_stop_times() const;
    const StopTime *get_next_stop(double lat, double long_) const;
    const StopTime *get_theorical_stop() const;
    const std::vector<size_t> *get_cache_nearest_stop() const;
    const std::vector<size_t> *get_cache_nearest_shape() const;
    const std::vector<double> *get_cache_stop_distances() const;
    const std::vector<Point *> *get_shape() const;
private:
    std::string trip_id;
    std::string route_id;
    std::string route_short_name;
    std::string route_long_name;
    std::vector<Point *> *shape;
    std::vector<StopTime *> *stop_times;
    std::vector<size_t> cache_nearest_stop;
    std::vector<size_t> cache_nearest_shape;
    std::vector<double> cache_stop_distances;
};

#endif