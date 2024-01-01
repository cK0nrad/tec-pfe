#ifndef SQLITEIMP
#define SQLITEIMP
extern "C"
{
#include "../../sqlite/sqlite3.h"
}
#endif

#include "request_manager.hpp"
#include "trip_data.hpp"
#include "afficheur_data.hpp"

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
    std::unique_lock<std::shared_mutex> lock(mutex);
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

// gonna iterate linearly over the results anyway so no need of vec
std::list<AfficheurData *> *RequstManager::get_afficheur(const char *like_name)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    std::list<AfficheurData *> *afficheurs = new std::list<AfficheurData *>();
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
        const char *id_raw = (const char *)(sqlite3_column_text(stmt, 0));
        const char *text_raw = (const char *)(sqlite3_column_text(stmt, 1));
        std::string line;
        if (sqlite3_column_type(stmt, 2) == SQLITE_NULL)
        {
            line = std::string("");
        }
        else
        {
            const char *line_raw = (const char *)(sqlite3_column_text(stmt, 2));
            line = std::string(line_raw);
        }

        std::string id(id_raw);
        std::string text(text_raw);

        AfficheurData *afficheur = new AfficheurData(id, text, line);
        afficheurs->push_back(afficheur);
    }
    sqlite3_finalize(stmt);
    return afficheurs;
}

TripData *RequstManager::get_trip(const char *s_trip_id)
{
    std::unique_lock<std::shared_mutex> lock(mutex);

    size_t input_size = strlen(s_trip_id);
    if (!is_open || !input_size || input_size > 15)
    {
        return nullptr;
    }

    char sql[256];
    // avoid * since it can (unlikely) change order
    sprintf(sql, "SELECT trip_id, route_id, shape_id, afficheur_id, route_short_name, route_long_name FROM(SELECT trips.*, routes.* FROM trips JOIN routes ON routes.route_id = trips.route_id WHERE trip_id = \"%s\" LIMIT 1);", s_trip_id);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        return nullptr;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        return nullptr;
    }

    const char *raw_trip_id = (const char *)sqlite3_column_text(stmt, 0);
    const char *raw_route_id = (const char *)sqlite3_column_text(stmt, 1);
    const char *raw_shape_id = (const char *)sqlite3_column_text(stmt, 2);
    const char *raw_afficheur_id = (const char *)sqlite3_column_text(stmt, 3);
    const char *raw_route_short_name = (const char *)sqlite3_column_text(stmt, 4);
    const char *raw_route_long_name = (const char *)sqlite3_column_text(stmt, 5);

    std::string trip_id(raw_trip_id);
    std::string route_id(raw_route_id);
    std::string shape_id(raw_shape_id);
    std::string afficheur_id(raw_afficheur_id);
    std::string route_short_name(raw_route_short_name);
    std::string route_long_name(raw_route_long_name);

    std::vector<StopTime *> *stop_times = get_stop_times(trip_id.c_str());
    std::vector<Point *> *shape = get_shape(shape_id.c_str());

    TripData *trip_data = new TripData(
        trip_id,
        route_id,
        afficheur_id,
        route_short_name,
        route_long_name,
        shape,
        stop_times);

    sqlite3_finalize(stmt);
    return trip_data;
}

std::vector<StopTime *> *RequstManager::get_stop_times(const char *trip_id)
{
    std::vector<StopTime *> *stop_times = new std::vector<StopTime *>();
    size_t input_size = strlen(trip_id);
    if (!is_open || !input_size || input_size > 15)
    {
        return stop_times;
    }

    char sql[256];
    // avoid * since it can (unlikely) change order
    sprintf(sql, "SELECT stop_id, stop_name, lat, lon, arrival_time FROM (SELECT stops.*, stops_times.* FROM stops JOIN stops_times ON stops.stop_id = stops_times.stop_id WHERE trip_id = \"%s\" ORDER BY arrival_time ASC)", trip_id);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        return stop_times;
    }

    StopTime *current = nullptr;
    StopTime *previous = nullptr;
    size_t idx = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        const char *raw_stop_id = (const char *)sqlite3_column_text(stmt, 0);
        const char *raw_stop_name = (const char *)sqlite3_column_text(stmt, 1);
        double lat = sqlite3_column_double(stmt, 2);
        double lon = sqlite3_column_double(stmt, 3);
        int arrival_time = sqlite3_column_int(stmt, 4);

        std::string stop_id(raw_stop_id);
        std::string stop_name(raw_stop_name);
        current = new StopTime{stop_id, stop_name, nullptr, lat, lon, false, arrival_time, idx++};

        if (previous)
            previous->next = current;

        previous = current;
        stop_times->push_back(current);
    }
    if (current)
        current->is_end = true;

    sqlite3_finalize(stmt);
    return stop_times;
}

std::vector<Point *> *RequstManager::get_shape(const char *shape_id)
{
    std::vector<Point *> *shape_data = new std::vector<Point *>();
    size_t input_size = strlen(shape_id);
    if (!is_open || !input_size || input_size > 15)
        return shape_data;

    char sql[128];
    // avoid * since it can (unlikely) change order
    sprintf(sql, "SELECT lat, lon  FROM shape WHERE shape_id = \"%s\" ORDER BY seq ASC", shape_id);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        return shape_data;
    }

    Point *current = nullptr;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        double lat = sqlite3_column_double(stmt, 0);
        double lon = sqlite3_column_double(stmt, 1);
        current = new Point{lat, lon};
        shape_data->push_back(current);
    }
    sqlite3_finalize(stmt);
    return shape_data;
}
