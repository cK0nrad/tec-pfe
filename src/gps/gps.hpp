#ifndef GPSD
#define GPSD

#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <list>

class Store;

struct StopTime;
struct Point;
class GPS
{
public:
    GPS(Store *store);
    ~GPS();
    void start();
    void stop();

private:
    Store *store;
    void main_loop();
    void await();
    void run();
    double get_average();
    void add_to_average(Point point);
    
    //easier to popup than a vector
    std::list<Point> gps_position;

    int stop_flag = 0;
    std::thread thread;
    int tc;
    int sock;
};
#endif