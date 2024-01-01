/*
    Module to send all the data to a TCP server
*/

#include "sync.hpp"
#include <thread>
#include "../store.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <chrono>

#define SLEEP_TIME 1 //[s]

static void sendInt(int socket, int value)
{
    send(socket, &value, sizeof(value), 0);
}

static void sendFloat(int socket, float value)
{
    send(socket, &value, sizeof(value), 0);
}

static void sendString(int socket, const std::string &value)
{
    int length = value.size();
    sendInt(socket, length);
    send(socket, value.c_str(), length, 0);
}

SyncClient::SyncClient(Store *store) : stop_flag(false), store(store), sock(0)
{
}

SyncClient::~SyncClient()
{
    stop();
    await();
}

void SyncClient::start()
{
    thread = std::thread(&SyncClient::main_loop, this);
}

void SyncClient::main_loop()
{
    while (!stop_flag)
    {
        run();
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    }
}

void SyncClient::await()
{
    if (thread.joinable())
        thread.join();
}

void SyncClient::run()
{
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "Socket creation error" << std::endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cout << "Invalid address / Address not supported" << std::endl;
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "Connection Failed" << std::endl;
        return;
    }

    sendFloat(sock, 3.14f);
    sendString(sock, "Hello from C++");

    close(sock);
}

void SyncClient::stop()
{
    stop_flag = 1;
}
