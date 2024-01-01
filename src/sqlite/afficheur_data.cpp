#include "afficheur_data.hpp"
#include <cstdio>
AfficheurData::AfficheurData(std::string id, std::string text, std::string line) : id(id), text(text), line(line) {}

AfficheurData::AfficheurData(AfficheurData *aff)
{
    id = std::string(aff->get_id().c_str());
    text = std::string(aff->get_text().c_str());
    line = std::string(aff->get_line().c_str());
}

char *AfficheurData::get_formatter() const
{
    char *formated = (char *)malloc(sizeof(char) * (line.length() + 3 + text.length() + 1));
    if (!formated)
        throw std::bad_alloc();

    strcpy(formated, get_line().c_str());
    strcat(formated, " - ");
    strcat(formated, get_text().c_str());
    return formated;
}

char *AfficheurData::get_line_formatter() const
{
    char *formated = (char *)malloc(sizeof(char) * (1 + id.length() + 2 + line.length() + 2 + text.length() + 1));
    if (!formated)
        throw std::bad_alloc();
    strcpy(formated, "[");
    strcat(formated, id.c_str());
    strcat(formated, "] ");
    strcat(formated, line.c_str());
    strcat(formated, ": ");
    strcat(formated, text.c_str());
    return formated;
}

const std::string &AfficheurData::get_id() const { return id; }
const std::string &AfficheurData::get_text() const { return text; }
const std::string &AfficheurData::get_line() const { return line; }