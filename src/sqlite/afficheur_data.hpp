#ifndef AFFICHEUR_DATA
#define AFFICHEUR_DATA
#include <cstring>
#include <cstdlib>
#include <string>
class AfficheurData
{
public:
    AfficheurData(std::string id, std::string text, std::string line);
    AfficheurData(AfficheurData *aff);

    const std::string &get_id() const;
    const std::string &get_text() const;
    const std::string &get_line() const;
    char *get_formatter() const;
    char *get_line_formatter() const;

private:
    std::string id;
    std::string text;
    std::string line;
};

#endif