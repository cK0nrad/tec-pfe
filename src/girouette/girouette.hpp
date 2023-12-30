#ifndef GIROUETTE
#define GIROUETTE

#include "../store.hpp"
#include <thread>
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