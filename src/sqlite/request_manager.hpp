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

class RequstManager
{
public:
    RequstManager();
    ~RequstManager();
    bool open();
    std::list<AfficheurData *> *get_afficheur(const char *like_name);

private:
    bool close();
    mutable std::shared_mutex mutex;
    bool is_open;
    sqlite3 *db;
    char *errMsg;
    int rc;
};

#endif