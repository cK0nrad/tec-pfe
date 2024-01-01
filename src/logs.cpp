#include "logs.hpp"
#include <chrono>
#include <iostream>
void error_log(const char *msg)
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char time[32];
    strftime(time, sizeof(time), "%d/%m/%Y %H:%M:%S", std::localtime(&in_time_t));
    std::cout << "[" << time << "] ";
    std::cout << "[ERROR] ";
    std::cout << msg << std::endl;
    exit(0);
}

void info_log(const char *msg)
{
    if (LOG_LEVEL < 1)
        return;
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    char time[32];
    strftime(time, sizeof(time), "%d/%m/%Y %H:%M:%S", std::localtime(&in_time_t));
    std::cout << "[" << time << "] ";
    std::cout << "[INFO] ";
    std::cout << msg << std::endl;
}
