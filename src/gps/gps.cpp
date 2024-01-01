#define SOCKET_PATH "/tmp/tec_gps.socket"
#include "gps.hpp"
#include "../store.hpp"
#include "../sqlite/trip_data.hpp"
#include "../sqlite/afficheur_data.hpp"
#include "../sqlite/request_manager.hpp"


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

GPS::GPS(Store *store) : store(store), stop_flag(0), tc(0)
{
}

void GPS::start()
{
    thread = std::thread(&GPS::main_loop, this);
}

GPS::~GPS()
{
    stop();
    await();
}

void GPS::main_loop()
{
    // loop endlessly to prevent the thread from exiting at error
    while (!stop_flag)
    {
        run();
        tc -= 1;
        store->set_gps_status(GPSStatus::ERROR);

        // If there's no trip, no need to continue
        if (!store->is_line_active())
        {
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

        char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
        if (!buffer)
            throw std::bad_alloc();

        time_t st = next_stop->arrival_time;
        struct tm *rt_next_stop_time = std::gmtime(&st);
        std::strftime(buffer, CLOCK_TEXT_LENGTH, "%H:%M", rt_next_stop_time);
        store->set_next_stop(next_stop->stop_name, std::string(buffer));
        free(buffer);
        store->set_delay(std::string("?"));

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
        std::cout << "Socket creation error: " << errno << std::endl;
        store->set_gps_status(GPSStatus::ERROR);
        return;
    }

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "Connection Failed" << std::endl;
        store->set_gps_status(GPSStatus::ERROR);
        return;
    }

    double values[2];

    // Loop to continuously receive values
    while (true)
    {
        if (stop_flag)
            break;

        ssize_t read_size = read(sock, values, sizeof(values));
        if (read_size <= 0)
        {
            std::cout << "Server closed the connection or error occurred" << std::endl;
            store->set_gps_status(GPSStatus::ERROR);
            break;
        }

        store->set_gps_position(values[0], values[1]);
        store->set_gps_status(GPSStatus::RUNNING);

        if (store->is_line_active())
        {
            TripData trip_data = store->get_current_trip();

            const StopTime *next_stop = trip_data.get_next_stop(values[0], values[1]);
            if (next_stop == nullptr)
                continue;

            char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
            if (!buffer)
                throw std::bad_alloc();

            time_t st = next_stop->arrival_time;
            struct tm *rt_next_stop_time = std::gmtime(&st);
            std::strftime(buffer, CLOCK_TEXT_LENGTH, "%H:%M", rt_next_stop_time);
            store->set_next_stop(next_stop->stop_name, std::string(buffer));
            free(buffer);

            // Get delay by regression
            // Knowning space between two stops and time between two stops
            // we can estimate the delay at any time
            // using shape instead of stops location to get more accurate results
            // it's run every second so it's not a big deal

            const StopTime *th = trip_data.get_theorical_stop();
            if (th == nullptr)
                continue;

            std::time_t rawtime = std::time(nullptr);
            struct tm *timeinfo = std::localtime(&rawtime);
            int current_time = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;

            const std::vector<size_t> *cache_nearest_shape = trip_data.get_cache_nearest_shape();
            const std::vector<size_t> *cache_nearest_stop = trip_data.get_cache_nearest_stop();
            const std::vector<double> *cache_distance = trip_data.get_cache_stop_distances();
            const std::vector<StopTime *> *stop_times = trip_data.get_stop_times();

            if (!cache_nearest_shape || !cache_nearest_stop || !cache_distance || !stop_times)
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
            printf("Nearest point: %zu\n", nearest_point);

            // If the trip didn't already start
            if (!th->idx)
            {
                printf("Trip not started\n");
                // 1. Get the delay between the current time and the theorical arrival time
                time_t delay = th->arrival_time - current_time;

                // 2. Get the delay the next stop and the theorical arrival time
                time_t delay2 = next_stop->arrival_time - th->arrival_time;

                printf("Theorical stop: %s\n", th->stop_name.c_str());
                time_t theorical_at = th->arrival_time;
                struct tm *theorical_at_tm = std::gmtime(&theorical_at);
                printf("Theorical arrival time: %d:%d:%d\n", theorical_at_tm->tm_hour, theorical_at_tm->tm_min, theorical_at_tm->tm_sec);

                printf("Current stop: %s\n", next_stop->stop_name.c_str());
                struct tm *rt_next_stop_time = std::gmtime(&st);
                printf("Arrival time: %d:%d:%d\n", rt_next_stop_time->tm_hour, rt_next_stop_time->tm_min, rt_next_stop_time->tm_sec);

                // 3. Get the total delay
                time_t total_delay = delay + delay2;
                char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
                if (!buffer)
                    throw std::bad_alloc();

                if (total_delay < 0)
                {
                    total_delay = -total_delay;
                }

                struct tm *timeinfo = std::gmtime(&total_delay);
                std::strftime(buffer, CLOCK_TEXT_LENGTH, "-%H:%M", timeinfo);

                store->set_delay(std::string(buffer));
                free(buffer);

                printf("\n");
                printf("\n");
                continue;
            }
            double avg = 180.0;

            // TODO: Reduce dry code & reformat

            // Three cases:
            // Assuming that n is our next stop
            // 1. The bus is before the n-1 stop (the bus is late)
            if (next_stop->idx < th->idx - 1)
            {
                printf("Bus is late\n");
                // 1.1 Track current delay to next stop as absolute delay
                time_t theorical_at = th->arrival_time;
                struct tm *theorical_at_tm = std::gmtime(&theorical_at);
                int theorical_time = theorical_at_tm->tm_hour * 3600 + theorical_at_tm->tm_min * 60 + theorical_at_tm->tm_sec;
                int absolute_delay = current_time - theorical_time;

                // 1.2 Track the distance between the bus and the n-1 stop
                size_t theorical_stop_idx = th->idx - 1;
                double distance = .0;
                Point *previous = (*shape)[nearest_point];
                for (size_t i = nearest_point + 1; i < (*cache_nearest_stop)[theorical_stop_idx]; i++)
                {
                    Point *current = (*shape)[i];
                    distance += earth_distance(previous->lat, previous->long_, current->lat, current->long_);
                    previous = current;
                }
                distance *= 1000;

                // 1.3 Track the distance between the n-1 and the n stop
                double distance2 = (*cache_distance)[th->idx] - (*cache_distance)[th->idx - 1];
                distance2 *= 1000;

                // 1.4 Track the time between the n-1 and the n stop
                int dt = (*stop_times)[th->idx]->arrival_time - (*stop_times)[th->idx - 1]->arrival_time;
                double dmdt = distance2 / (double)dt;

                // 1.5 Get virtual distance between the n-1 and theorical stop
                if (th->is_end && current_time > th->arrival_time)
                {
                    current_time = th->arrival_time;
                }
                double virtual_distance = (current_time - (*stop_times)[th->idx - 1]->arrival_time) * dmdt;

                // 1.6 Get virtual delay
                double virtual_delay = (virtual_distance + distance) / avg + absolute_delay;

                // 1.7 Scheduled delay i.e. independent delay based on schedule
                // This is the delay that the bus have when it is late at another stop
                int scheduled_delay = th->arrival_time - next_stop->arrival_time;
                double total_delay = virtual_delay + scheduled_delay;
                time_t total_delay_int = (int)total_delay;

                char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
                if (!buffer)
                    throw std::bad_alloc();

                total_delay_int = total_delay_int < 0 ? -total_delay_int : total_delay_int;
                struct tm *timeinfo = std::gmtime(&total_delay_int);
                std::strftime(buffer, CLOCK_TEXT_LENGTH, "+%H:%M", timeinfo);

                store->set_delay(std::string(buffer));
                free(buffer);
            }
            // 2. The bus is between n-1 and n stop (the bus is on time or slightly delayed (total trip hours delayed))
            else if (next_stop->idx == th->idx)
            {
                printf("Bus is on time or slightly delayed\n");
                // 1.1 Track current delay to next stop as absolute delay
                time_t theorical_at = th->arrival_time;
                struct tm *theorical_at_tm = std::gmtime(&theorical_at);
                int theorical_time = theorical_at_tm->tm_hour * 3600 + theorical_at_tm->tm_min * 60 + theorical_at_tm->tm_sec;
                int absolute_delay = current_time - theorical_time;

                // 2.2 Track the distance between the bus and the n-1 stop
                size_t theorical_stop_idx = th->idx - 1;
                double distance = .0;
                Point *previous = (*shape)[nearest_point];
                for (size_t i = nearest_point + 1; i < (*cache_nearest_stop)[theorical_stop_idx]; i++)
                {
                    Point *current = (*shape)[i];
                    distance += earth_distance(previous->lat, previous->long_, current->lat, current->long_);
                    previous = current;
                }
                distance *= 1000.0;

                // 2.3 Track the distance between the n-1 and the n stop
                double distance2 = (*cache_distance)[th->idx] - (*cache_distance)[th->idx - 1];
                distance2 *= 1000.0;

                // 2.4 Track the time between the n-1 and the n stop
                int dt = (*stop_times)[th->idx]->arrival_time - (*stop_times)[th->idx - 1]->arrival_time;

                // 2.5 Get virtual distance between the n-1 and theorical stop
                if (th->is_end && current_time > th->arrival_time)
                {
                    current_time = th->arrival_time;
                }

                double dmdt = distance2 / (double)dt;
                double virtual_distance = (current_time - (*stop_times)[th->idx - 1]->arrival_time) * dmdt;

                // 2.6 Get virtual delay
                double virtual_delay = virtual_distance / avg + absolute_delay;
                time_t virtual_delay_int = (int)virtual_delay;

                char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
                if (!buffer)
                    throw std::bad_alloc();

                if (virtual_delay_int < 0)
                {
                    virtual_delay_int = -virtual_delay_int;
                    struct tm *timeinfo = std::gmtime(&virtual_delay_int);
                    std::strftime(buffer, CLOCK_TEXT_LENGTH, "-%H:%M", timeinfo);
                }
                else
                {
                    virtual_delay_int = -virtual_delay_int;
                    struct tm *timeinfo = std::gmtime(&virtual_delay_int);
                    std::strftime(buffer, CLOCK_TEXT_LENGTH, "+%H:%M", timeinfo);
                }

                store->set_delay(std::string(buffer));
                free(buffer);
            }
            // 3. The bus is after the n stop (the bus is early)
            else
            {
                printf("Bus is early\n");
                // Should never happens
                if (!next_stop->idx)
                {
                    continue;
                }
                // 3.1 Track current delay to next stop as absolute delay
                time_t theorical_at = th->arrival_time;
                struct tm *theorical_at_tm = std::gmtime(&theorical_at);
                int theorical_time = theorical_at_tm->tm_hour * 3600 + theorical_at_tm->tm_min * 60 + theorical_at_tm->tm_sec;
                int absolute_delay = current_time - theorical_time;

                // 3.2 Track the distance between the bus and the next stop - 1
                size_t theorical_stop_idx = next_stop->idx - 1;
                double distance = .0;
                Point *previous = (*shape)[nearest_point];
                for (size_t i = nearest_point + 1; i < (*cache_nearest_stop)[theorical_stop_idx]; i++)
                {
                    Point *current = (*shape)[i];
                    distance += earth_distance(previous->lat, previous->long_, current->lat, current->long_);
                    previous = current;
                }
                distance *= 1000.0;

                // 3.3 Track the distance between the next stop - 1 and the next stop
                double distance2 = (*cache_distance)[next_stop->idx] - (*cache_distance)[next_stop->idx - 1];
                distance2 *= 1000.0;

                // 3.4 Track the time between the next stop - 1 and the next stop
                int dt = (*stop_times)[next_stop->idx]->arrival_time - (*stop_times)[next_stop->idx - 1]->arrival_time;

                // 3.5 Get virtual distance between the next stop - 1 and theorical stop
                if (th->is_end && current_time > th->arrival_time)
                {
                    current_time = th->arrival_time;
                }

                double dmdt = distance2 / (double)dt;
                double virtual_distance = (current_time - (*stop_times)[next_stop->idx - 1]->arrival_time) * dmdt;

                // 3.6 Get total distance between stop - 1 and theorical stop
                double distance3 = (*cache_distance)[th->idx] - (*cache_distance)[next_stop->idx - 1];
                distance3 *= 1000.0;

                // 3.6 Get virtual delay
                double virtual_delay = (virtual_distance + distance3) / avg + absolute_delay;

                // 3.7 Scheduled delay i.e. independent delay based on schedule
                int scheduled_delay = th->arrival_time - next_stop->arrival_time;

                double total_delay = virtual_delay + scheduled_delay;
                time_t total_delay_int = (int)total_delay;

                char *buffer = (char *)malloc(CLOCK_TEXT_LENGTH * sizeof(char));
                if (!buffer)
                    throw std::bad_alloc();

                total_delay_int = total_delay_int < 0 ? -total_delay_int : total_delay_int;
                struct tm *timeinfo = std::gmtime(&total_delay_int);
                std::strftime(buffer, CLOCK_TEXT_LENGTH, "-%H:%M", timeinfo);

                store->set_delay(std::string(buffer));
                free(buffer);
            }
        }

        std::cout << "GPS is at pos: " << values[0] << ", " << values[1] << "\n"
                  << std::endl;
    }

    close(sock);
    return;
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
