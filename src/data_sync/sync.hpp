#ifndef SYNC_CLIENT
#define SYNC_CLIENT

#include <thread>

class Store;

class SyncClient
{
public:
    SyncClient(Store *store);
    ~SyncClient();

    void start();
    void stop();

private:
    void main_loop();
    void await();
    void run();
    bool stop_flag;
    std::thread thread;
    Store *store;
    int sock;
};

#endif