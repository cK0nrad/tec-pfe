#ifndef GPSD
#define GPSD
#include "../store.hpp"
#include <thread>

class GPS
{
public:
    GPS(Store *store);
    void start();
    void stop();
    void await();

private:
    Store *store;
    void main_loop();
    void run();
    int stop_flag = 0;
    std::thread thread;
    int tc;
};
#endif