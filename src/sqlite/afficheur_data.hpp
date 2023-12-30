#ifndef AFFICHEUR_DATA
#define AFFICHEUR_DATA
#include <cstring>
#include <cstdlib>
class AfficheurData
{
public:
    AfficheurData(const char *id, const char *text, const char *line);
    AfficheurData(AfficheurData *aff);
    ~AfficheurData();



    const char *get_id() const;
    const char *get_text() const;
    const char *get_line() const;
    const char *get_formatter() const;

private:
    char *id;
    char *text;
    char *line;
};

#endif