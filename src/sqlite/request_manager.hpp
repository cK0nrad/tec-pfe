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

class AfficheurData
{
public:
    AfficheurData(const char *id, const char *text, const char *line) : id((char *)id), text((char *)text), line((char *)line) {}
    ~AfficheurData()
    {
        free((void *)id);
        free((void *)text);
        free((void *)line);
    }

    const char *get_id() const { return id; }
    const char *get_text() const { return text; }
    const char *get_line() const { return line; }
private:
    char *id;
    char *text;
    char *line;
};

class RequstManager
{
public:
    RequstManager();
    ~RequstManager();
    bool open();
    std::list<AfficheurData *> get_afficheur(const char *like_name);

private:
    bool close();
    mutable std::shared_mutex mutex;
    bool is_open;
    sqlite3 *db;
    char *errMsg;
    int rc;
};

#endif