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
#include "../sqlite/afficheur_data.hpp"
#include "../sqlite/trip_data.hpp"

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