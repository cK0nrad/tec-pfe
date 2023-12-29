#ifndef SQLITEIMP
#define SQLITEIMP
extern "C"
{
#include "../../sqlite/sqlite3.h"
}
#endif

#include "request_manager.hpp"
#include <list>
#include <shared_mutex>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <cstdlib>

RequstManager::RequstManager() : is_open(false), db(nullptr), errMsg(nullptr), rc(0)
{
}

RequstManager::~RequstManager()
{
    if (!is_open)
        return;
    close();
}

bool RequstManager::open()
{
    rc = sqlite3_open("./internal_db.db", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    else
    {
        fprintf(stderr, "DB Loaded\n");
        is_open = true;
        return true;
    }
}

bool RequstManager::close()
{
    if (!is_open)
        return false;

    sqlite3_close(db);
    return true;
}

// gonna iterate linearly over the results so no need of vec
std::list<AfficheurData *> RequstManager::get_afficheur(const char *like_name)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    std::list<AfficheurData *> afficheurs = std::list<AfficheurData *>();
    size_t input_size = strlen(like_name);
    if (!is_open || !input_size || input_size > 15)
        return afficheurs;

    char sql[128];
    // avoid * since it can (unlikely) change order
    sprintf(sql, "SELECT id, text, line_number FROM afficheurs WHERE id LIKE \"%s%%\" LIMIT 20;", like_name);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        return afficheurs;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *id = (const char *)sqlite3_column_text(stmt, 0);
        const char *text = (const char *)sqlite3_column_text(stmt, 1);
        const char *line = (const char *)sqlite3_column_text(stmt, 2);

        const char *id_ = (const char *)malloc(sizeof(char) * (strlen(id) + 1));
        const char *text_ = (const char *)malloc(sizeof(char) * (strlen(text) + 1));
        const char *line_ = (const char *)malloc(sizeof(char) * (strlen(line) + 1));

        strcpy((char *)id_, id);
        strcpy((char *)text_, text);
        strcpy((char *)line_, line);

        AfficheurData *afficheur = new AfficheurData(id_, text_, line_);
        afficheurs.push_back(afficheur);
    }
    sqlite3_finalize(stmt);
    return afficheurs;
}