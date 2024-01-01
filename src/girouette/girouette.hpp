#ifndef GIROUETTE
#define GIROUETTE

#include <thread>

class Store;

class Girouette
{
public:
    Girouette(Store *store);
    ~Girouette();
    void start();
    void main_loop();
    void stop();
    void await();

private:
    bool stop_flag;
    Store *store;
    size_t current_index;
    std::thread thread;
};

#endif