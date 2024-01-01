#include "trip_data.hpp"
#include "request_manager.hpp"

#include <vector>
#include <iostream>
#include <iomanip>



double earth_distance(double lat1, double lon1, double lat2, double lon2)
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

double sqr_distance(double lat1, double lon1, double lat2, double lon2)
{
    double dLat = (lat2 - lat1);
    double dLon = (lon2 - lon1);
    return dLat * dLat + dLon * dLon;
}

TripData::TripData(const TripData *trip) : trip_id(trip->trip_id),
                                           route_id(trip->route_id),
                                           afficheur_id(trip->afficheur_id),
                                           route_short_name(trip->route_short_name),
                                           route_long_name(trip->route_long_name)
{
    shape = new std::vector<Point *>();
    for (auto point : *trip->shape)
    {
        shape->push_back(new Point{point->lat, point->long_});
    }

    stop_times = new std::vector<StopTime *>();
    StopTime *previous = nullptr;
    for (auto stop_time : *trip->stop_times)
    {
        StopTime *new_stop_time = new StopTime();
        new_stop_time->stop_id = stop_time->stop_id;
        new_stop_time->stop_name = stop_time->stop_name;
        new_stop_time->position = Point{stop_time->position.lat, stop_time->position.long_};
        new_stop_time->is_end = stop_time->is_end;
        new_stop_time->arrival_time = stop_time->arrival_time;
        new_stop_time->idx = stop_time->idx;

        if (previous != nullptr)
        {
            previous->next = new_stop_time;
        }
        previous = new_stop_time;
        stop_times->push_back(new_stop_time);
    }

    cache_stop_distances = std::vector<double>(stop_times->size(), 0.0);
    cache_nearest_stop.reserve(stop_times->size());
    cache_nearest_shape.reserve(shape->size());

    // Make caches for reverse search (nearest stop)
    // Nearest shpe
    // i.e for each stop, find nearest shape
    size_t last_index = 0;
    for (size_t j = 0; j < stop_times->size(); j++)
    {
        double smallest = INFINITY;
        for (size_t i = last_index; i < shape->size(); i++)
        {
            double distance = sqr_distance(
                stop_times->at(j)->position.lat,
                stop_times->at(j)->position.long_,
                shape->at(i)->lat,
                shape->at(i)->long_);
            if (distance < smallest)
            {
                smallest = distance;
                last_index = i;
            }
        }
        cache_nearest_stop.push_back(last_index);
    }

    // Nearrest stop
    // i.e for each point in shape, find nearest stop
    last_index = 0;
    for (size_t j = 0; j < shape->size(); j++)
    {
        double smallest = INFINITY;
        for (size_t i = 0; i < stop_times->size(); i++)
        {
            double distance = sqr_distance(
                stop_times->at(i)->position.lat,
                stop_times->at(i)->position.long_,
                shape->at(j)->lat,
                shape->at(j)->long_);
            if (distance < smallest)
            {
                smallest = distance;
                last_index = i;
            }
        }
        if (cache_nearest_stop[last_index] <= j && last_index < stop_times->size() - 1)
        {
            last_index += 1;
        }

        cache_nearest_shape.push_back(last_index);
    }

    size_t stop_index = 0;
    Point *last_point = shape->at(0);
    for (size_t i = 0; i < shape->size() - 1; i++)
    {
        Point *current_point = shape->at(i);
        cache_stop_distances[stop_index] += earth_distance(
            last_point->lat,
            last_point->long_,
            current_point->lat,
            current_point->long_);
        last_point = current_point;

        if (stop_index < shape->size() - 1 && cache_nearest_stop[stop_index] <= i)
        {
            stop_index++;
        }
    }
}

TripData::TripData(
    std::string trip_id,
    std::string route_id,
    std::string afficheur_id,
    std::string route_short_name,
    std::string route_long_name,
    std::vector<Point *> *shape,
    std::vector<StopTime *> *stop_times) : trip_id(trip_id),
                                           route_id(route_id),
                                           afficheur_id(afficheur_id),
                                           route_short_name(route_short_name),
                                           route_long_name(route_long_name),
                                           shape(shape),
                                           stop_times(stop_times)
{
    cache_stop_distances = std::vector<double>(stop_times->size(), 0.0);
    cache_nearest_stop.reserve(stop_times->size());
    cache_nearest_shape.reserve(shape->size());

    // Make caches for reverse search (nearest stop)
    // Nearest shpe
    // i.e for each stop, find nearest shape
    size_t last_index = 0;
    for (size_t j = 0; j < stop_times->size(); j++)
    {
        double smallest = INFINITY;
        for (size_t i = last_index; i < shape->size(); i++)
        {
            double distance = sqr_distance(
                stop_times->at(j)->position.lat,
                stop_times->at(j)->position.long_,
                shape->at(i)->lat,
                shape->at(i)->long_);
            if (distance < smallest)
            {
                smallest = distance;
                last_index = i;
            }
        }
        cache_nearest_stop.push_back(last_index);
    }

    // Nearrest stop
    // i.e for each point in shape, find nearest stop
    last_index = 0;
    for (size_t j = 0; j < shape->size(); j++)
    {
        double smallest = INFINITY;
        for (size_t i = 0; i < stop_times->size(); i++)
        {
            double distance = sqr_distance(
                stop_times->at(i)->position.lat,
                stop_times->at(i)->position.long_,
                shape->at(j)->lat,
                shape->at(j)->long_);
            if (distance < smallest)
            {
                smallest = distance;
                last_index = i;
            }
        }
        if (cache_nearest_stop[last_index] <= j && last_index < stop_times->size() - 1)
        {
            last_index += 1;
        }

        cache_nearest_shape.push_back(last_index);
    }

    size_t stop_index = 0;
    Point *last_point = shape->at(0);
    for (size_t i = 0; i < shape->size() - 1; i++)
    {
        Point *current_point = shape->at(i);
        cache_stop_distances[stop_index] += earth_distance(
            last_point->lat,
            last_point->long_,
            current_point->lat,
            current_point->long_);
        last_point = current_point;

        if (stop_index < shape->size() - 1 && cache_nearest_stop[stop_index] <= i)
        {
            stop_index++;
        }
    }
}

TripData::~TripData()
{
    for (auto stop_time : *stop_times)
    {
        delete stop_time;
    }
    delete stop_times;
    for (auto point : *shape)
    {
        delete point;
    }
    delete shape;
}

const std::vector<Point *> *TripData::get_trip() const
{
    return shape;
}

const std::vector<StopTime *> *TripData::get_stop_times() const
{
    return stop_times;
}

const StopTime *TripData::get_next_stop(double lat, double long_) const
{
    size_t nearest_shape_index = 0;
    // O(n) search ~> fast enough
    double smallest = INFINITY;
    for (size_t i = 0; i < shape->size(); i++)
    {
        double distance = sqr_distance(
            (*shape)[i]->lat,
            (*shape)[i]->long_,
            lat,
            long_);

        if (distance < smallest)
        {
            nearest_shape_index = i;
            smallest = distance;
        }
    }

    size_t nearest_stop_index = cache_nearest_shape[nearest_shape_index];
    if (nearest_stop_index == 0)
        return stop_times->at(0);

    if (nearest_stop_index >= stop_times->size())
        return stop_times->at(cache_nearest_shape[stop_times->size() - 1]);

    return stop_times->at(cache_nearest_shape[nearest_shape_index]);
}

const StopTime *TripData::get_theorical_stop() const
{
    std::time_t rawtime = std::time(nullptr);
    struct tm *timeinfo = std::localtime(&rawtime);
    int current_time = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;

    size_t i = 0;
    for (i = 0; i < stop_times->size() - 1 && !(stop_times->at(i)->arrival_time >= current_time); i++)
        ;
    return stop_times->at(i);
}

const std::vector<size_t> *TripData::get_cache_nearest_stop() const
{
    return &cache_nearest_stop;
}

const std::vector<size_t> *TripData::get_cache_nearest_shape() const
{
    return &cache_nearest_shape;
}

const std::vector<double> *TripData::get_cache_stop_distances() const
{
    return &cache_stop_distances;
}

const std::vector<Point *> *TripData::get_shape() const
{
    return shape;
}

std::string TripData::get_trip_id() const
{
    return trip_id;
}

std::string TripData::get_afficheur_id() const
{
    return afficheur_id;
}


const StopTime *TripData::get_nth_stop(size_t idx) const
{
    if(idx >= stop_times->size())
        idx = stop_times->size() - 1;

    return stop_times->at(idx);
}