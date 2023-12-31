#include "afficheur_data.hpp"
#include <cstdio>
AfficheurData::AfficheurData(std::string id, std::string text, std::string line) : id(id), text(text), line(line) {}

AfficheurData::AfficheurData(AfficheurData *aff)
{
    char *id = (char *)malloc(sizeof(char) * (aff->get_id().length() + 1));
    char *text = (char *)malloc(sizeof(char) * (aff->get_text().length() + 1));
    char *line = (char *)malloc(sizeof(char) * (aff->get_line().length() + 1));
    if (!id || !text || !line)
        throw std::bad_alloc();

    strcpy(id, aff->get_id().c_str());
    strcpy(text, aff->get_text().c_str());
    strcpy(line, aff->get_line().c_str());

    this->id = id;
    this->text = text;
    this->line = line;
}

AfficheurData::~AfficheurData()
{
}

char *AfficheurData::get_formatter() const
{
    char *formated = (char *)malloc(sizeof(char) * (get_line().length() + 3 + get_text().length() + 1));
    if (!formated)
        throw std::bad_alloc();

    strcpy(formated, get_line().c_str());
    strcat(formated, " - ");
    strcat(formated, get_text().c_str());
    return formated;
}

std::string AfficheurData::get_id() const { return id; }
std::string AfficheurData::get_text() const { return text; }
std::string AfficheurData::get_line() const { return line; }