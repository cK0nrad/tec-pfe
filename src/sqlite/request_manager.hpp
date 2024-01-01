#ifndef REQUSET_MANAGER
#define REQUSET_MANAGER

#ifndef SQLITEIMP
#define SQLITEIMP
extern "C"
{
#include "../../sqlite/sqlite3.h"
}
#endif

#include <list>
#include <shared_mutex>
#include <cstring>
#include <vector>

class TripData;
class AfficheurData;

struct Point
{
    double lat;
    double long_;
};

struct StopTime
{
    std::string stop_id;
    std::string stop_name;
    StopTime *next;
    Point position;
    bool is_end;
    int arrival_time;
    size_t idx;
};

std::string get_arrival_time(const StopTime *stop_time);


class RequstManager
{
public:
    RequstManager();
    ~RequstManager();
    bool open();
    std::list<AfficheurData *> *get_afficheur(const char *like_name);
    TripData *get_trip(const char *trip_id);

private:
    std::vector<StopTime *> *get_stop_times(const char *trip_id);
    std::vector<Point *> *get_shape(const char *shape_id);

    bool close();
    mutable std::shared_mutex mutex;
    bool is_open;
    sqlite3 *db;
    char *errMsg;
    int rc;
};

#endif