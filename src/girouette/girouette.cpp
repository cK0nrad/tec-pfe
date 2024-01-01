#include "girouette.hpp"
#include "../store.hpp"
#include "../logs.hpp"

#include <chrono>

#define SLEEP_TIME 5

Girouette::Girouette(Store *store) : stop_flag(false), store(store)
{
}

Girouette::~Girouette()
{
    stop();
    await();
}

void Girouette::stop()
{
    stop_flag = true;
}

void Girouette::await()
{
    if (thread.joinable())
        thread.join();
}

void Girouette::start()
{
    thread = std::thread(&Girouette::main_loop, this);
}

void Girouette::main_loop()
{
    current_index = 0;
    
    info_log("Girouette thread started");
    while (!stop_flag)
    {
        if(current_index >= store->get_girouettes_size())
        {
            current_index = 0;
        }
        store->change_girouette(current_index);
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
        current_index++;
    }
}
