#ifndef GPSD
#define GPSD

#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

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
    
    int stop_flag = 0;
    std::thread thread;
    int tc;
    int sock;
};
#endif