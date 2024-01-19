#define SOCKET_PATH "/tmp/tec_gps.socket"
#include "gps.hpp"
#include "../store.hpp"
#include "../sqlite/trip_data.hpp"
#include "../sqlite/afficheur_data.hpp"
#include "../sqlite/request_manager.hpp"
#include "../logs.hpp"

#include <thread>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <math.h>

#define WAIT_AFTER_ERROR 1 //[s]
#define CLOCK_TEXT_LENGTH 7
#define AVERAGE_MAX_POINT 100

GPS::GPS(Store *store) : store(store), stop_flag(0), tc(0), sock(0)
{
}

void GPS::start()
{
    thread = std::thread(&GPS::main_loop, this);
}

GPS::~GPS()
{
    stop();
    info_log("Stopping GPS thread...");
    await();
}

void GPS::main_loop()
{
    // loop endlessly to prevent the thread from exiting at error
    while (!stop_flag)
    {
        run();
        tc -= 1;
        if (stop_flag)
            break;
        store->set_gps_status(GPSStatus::ERROR);

        // If there's no trip, no need to continue
        if (!store->is_line_active())
        {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_AFTER_ERROR));
            continue;
        }

        // If there's a trip, check if the trip is overwritten
        if (store->is_next_stop_overwrite())
        {
            store->refresh_delay();
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_AFTER_ERROR));
            continue;
        }

        TripData trip_data = store->get_current_trip();
        const StopTime *next_stop = trip_data.get_theorical_stop();
        if (next_stop == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::seconds(WAIT_AFTER_ERROR));
            continue;
        }

        store->set_stop_index(next_stop->idx, true);
        std::this_thread::sleep_for(std::chrono::seconds(WAIT_AFTER_ERROR));
    }
}

void GPS::run()
{
    // Only one instance of this thread should be running
    // in theory this should never happen but since variables are
    // not initialized thread safe, this is a workaround
    if (tc > 0)
        return;

    tc += 1;
    struct sockaddr_un serv_addr;

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        error_log("GPS: Socket creation error");
        store->set_gps_status(GPSStatus::ERROR);
        return;
    }

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error_log("GPS: Connection Failed");
        store->set_gps_status(GPSStatus::ERROR);
        return;
    }

    double values[2];

    // Loop to continuously receive values
    while (!stop_flag)
    {

        ssize_t read_size = read(sock, values, sizeof(values));
        if (read_size <= 0)
        {
            error_log("GPS: Error reading from socket");
            store->set_gps_status(GPSStatus::ERROR);
            break;
        }

        store->set_gps_position(values[0], values[1]);
        store->set_gps_status(GPSStatus::RUNNING);

        add_to_average(Point{values[0], values[1]});

        if (store->is_line_active())
        {
            TripData trip_data = store->get_current_trip();

            // Get next stop
            const StopTime *next_stop = trip_data.get_next_stop(values[0], values[1]);
            if (next_stop == nullptr)
                continue;

            store->set_stop_index(next_stop->idx, false);

            // Get current time
            char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
            if (!buffer)
                throw std::bad_alloc();

            time_t st = next_stop->arrival_time;
            struct tm *rt_next_stop_time = std::gmtime(&st);
            std::strftime(buffer, CLOCK_TEXT_LENGTH, "%H:%M", rt_next_stop_time);
            store->set_next_stop(next_stop->stop_name, std::string(buffer));
            free(buffer);

            // Get delay
            const StopTime *th = trip_data.get_theorical_stop();
            if (th == nullptr)
                continue;

            std::time_t rawtime = std::time(nullptr);
            struct tm *timeinfo = std::localtime(&rawtime);
            int current_time = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;

            const std::vector<size_t> *cache_nearest_stop = trip_data.get_cache_nearest_stop();
            const std::vector<double> *cache_distance = trip_data.get_cache_stop_distances();
            const std::vector<StopTime *> *stop_times = trip_data.get_stop_times();

            if (!cache_nearest_stop || !cache_distance || !stop_times)
                continue;

            // Here we are doing a nearest point search
            //  which is also done when we get the next stop
            //  so we could just used it, but nevermind
            const std::vector<Point *> *shape = trip_data.get_shape();

            size_t nearest_point = 0;
            double smallest = INFINITY;
            for (size_t i = 0; i < shape->size(); i++)
            {
                double distance = sqr_distance(
                    (*shape)[i]->lat,
                    (*shape)[i]->long_,
                    values[0],
                    values[1]);

                if (distance < smallest)
                {
                    nearest_point = i;
                    smallest = distance;
                }
            }

            // Get next stops time
            int next_stops_time[2] = {0, 0};

            if (next_stop->idx == 0)
            {
                next_stops_time[0] = next_stop->arrival_time;
                if (next_stop->next != nullptr)
                    next_stops_time[1] = next_stop->next->arrival_time;
                else
                    next_stops_time[1] = next_stop->arrival_time;
            }
            else
            {
                next_stops_time[0] = stop_times->at(next_stop->idx - 1)->arrival_time;
                next_stops_time[1] = next_stop->arrival_time;
            }
            // Get remaining distance (bus -> next_stop)
            double remaining_distance = .0;
            Point *previous = (*shape)[nearest_point];
            for (size_t i = nearest_point + 1; i < (*cache_nearest_stop)[next_stop->idx]; i++)
            {
                Point *current = (*shape)[i];
                remaining_distance += earth_distance(previous->lat, previous->long_, current->lat, current->long_);
                previous = current;
            }
            remaining_distance *= 1000;

            size_t next_stop_idx = next_stop->idx > 0 ? next_stop->idx - 1 : 0;
            double total_next_distance = (*cache_distance)[next_stop_idx];
            total_next_distance *= 1000;
            get_delay(current_time, next_stops_time, remaining_distance, total_next_distance);
        }
    }

    close(sock);
    return;
}

void GPS::get_delay(long current_time,
                    int next_stops_time[2],
                    double remaining_distance,
                    double total_next_distance)
{
    double time_to_next_stop = (double)(next_stops_time[1] - next_stops_time[0]);
    time_to_next_stop = (time_to_next_stop / total_next_distance) * remaining_distance;
    double delay = (double)current_time + time_to_next_stop - ((double)next_stops_time[1]);

    char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
    if (!buffer)
        throw std::bad_alloc();

    time_t total_delay = static_cast<long>(std::floor(delay < 0 ? -delay : delay));
    struct tm *timeinfo = std::gmtime(&total_delay);
    std::strftime(buffer, CLOCK_TEXT_LENGTH, delay < 0 ? "+%H:%M" : "-%H:%M", timeinfo);
    store->set_delay(std::string(buffer));
    free(buffer);
}

void GPS::await()
{
    if (thread.joinable())
        thread.join();
}

// no concurrent access so no need for rw lock
void GPS::stop()
{
    stop_flag = 1;
}

double GPS::get_average()
{
    if (gps_position.size() < 2)
        return 0.0;

    double total_dist = 0.0;
    Point previous = gps_position.front();
    for (auto it = gps_position.begin(); it != gps_position.end(); it++)
    {
        total_dist += earth_distance(previous.lat, previous.long_, it->lat, it->long_) * 1000;
        previous = *it;
    }

    return total_dist / (double)gps_position.size();
}

void GPS::add_to_average(Point point)
{
    if (gps_position.size() >= AVERAGE_MAX_POINT)
        gps_position.pop_front();

    gps_position.push_back(point);
}