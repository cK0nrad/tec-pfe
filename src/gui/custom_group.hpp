#ifndef TEC_GROUP
#define TEC_GROUP

#include <FL/Fl_Group.H>

class TECGroup : public Fl_Group
{
public:
    TECGroup(int x, int y, int w, int h, const char *l = nullptr);
};

#endif