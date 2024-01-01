/*
    Module to send all the data to a TCP server
*/

#include "sync.hpp"
#include <thread>
#include "../store.hpp"
#include "../sqlite/afficheur_data.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <poll.h>
#include "../logs.hpp"

#define SLEEP_TIME 1 //[s]

SyncClient::SyncClient(Store *store) : stop_flag(false), store(store), sock(0)
{
}

SyncClient::~SyncClient()
{
    stop();
    info_log("Stopping Sync thread...");
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
        if (stop_flag)
            break;
        sock = 0;
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
        error_log("SYNC: Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        error_log("SYNC: Invalid address / Address not supported");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error_log("SYNC: Connection Failed");
        return;
    }

    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLIN;

    int rc = 0;
    while (!stop_flag)
    {

        int ret = poll(&fds, 1, 1000);
        if (ret > 0)
        {
            error_log("SYNC: Error polling socket");
            break;
        }

        GpsState t = store->get_gps_state();
        double lat = t.get_lat();
        double lon = t.get_lon();
        size_t next_stop = store->get_next_stop_idx();
        const char *girouette = store->get_current_girouette().get_line_formatter();

        // forget about the \0 since it's not utf8
        size_t girouette_size = strlen(girouette);

        std::vector<char> buffer;
        buffer.resize(sizeof(lat) + sizeof(lon) + (girouette_size) + sizeof(next_stop));

        std::memcpy(buffer.data(), &lat, sizeof(lat));
        std::memcpy(buffer.data() + sizeof(lat), &lon, sizeof(lon));
        std::memcpy(buffer.data() + sizeof(lat) + sizeof(lon), &next_stop, sizeof(next_stop));
        std::memcpy(buffer.data() + sizeof(lat) + sizeof(lon) + sizeof(next_stop), girouette, girouette_size);

        rc = send(sock, buffer.data(), buffer.size(), 0);

        if (rc == -1)
        {
            error_log("SYNC: Error sending data");
            break;
        }
        free((void *)girouette);
        std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIME));
    }

    close(sock);
}

void SyncClient::stop()
{
    stop_flag = 1;
}
