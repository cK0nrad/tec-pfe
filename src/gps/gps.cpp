#define SOCKET_PATH "/tmp/tec_gps.socket"
#include "gps.hpp"

GPS::GPS(Store *store) : store(store)
{
    tc = 0;
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
        std::this_thread::sleep_for(std::chrono::seconds(5));
        store->set_gps_status(GPSStatus::ERROR);
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
        std::cout << "Socket creation error" << std::endl;
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

        std::cout << "GPS is at pos: " << values[0] << ", " << values[1] << std::endl;
    }

    close(sock);
    return;
}
void GPS::await()
{
    thread.join();
}

// no concurrent access so no need for rw lock
void GPS::stop()
{
    stop_flag = 1;
}
