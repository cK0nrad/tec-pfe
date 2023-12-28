#ifndef TYPES
#define TYPES

enum GPSStatus
{
    STARTING,
    RUNNING,
    ERROR
};

class GpsState
{
public:
    GpsState(double lat, double lon, GPSStatus status)
    {
        this->lat = lat;
        this->lon = lon;
        this->status = status;
    }

    double get_lat() const
    {
        return lat;
    };

    double get_lon() const
    {
        return lon;
    };

    GPSStatus get_status() const
    {
        return status;
    };

    void set_lat(double lat)
    {
        this->lat = lat;
    };

    void set_lon(double lon)
    {
        this->lon = lon;
    };

    void set_status(GPSStatus status)
    {
        this->status = status;
    };

private:
    double lat;
    double lon;
    GPSStatus status;
};

// non opaque for convenience
//  typedef struct gps_state_t
//  {
//      double lat;
//      double lon;
//      GPSStatus status;
//  } GpsState;

#endif