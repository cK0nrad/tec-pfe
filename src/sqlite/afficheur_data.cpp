#include "afficheur_data.hpp"
#include <cstdio>
AfficheurData::AfficheurData(const char *id, const char *text, const char *line) : id((char *)id), text((char *)text), line((char *)line) {}

AfficheurData::AfficheurData(AfficheurData *aff)
{
    char *id = (char *)malloc(sizeof(char) * (strlen(aff->get_id()) + 1));
    char *text = (char *)malloc(sizeof(char) * (strlen(aff->get_text()) + 1));
    char *line = (char *)malloc(sizeof(char) * (strlen(aff->get_line()) + 1));

    strcpy(id, aff->get_id());
    strcpy(text, aff->get_text());
    strcpy(line, aff->get_line());

    this->id = id;
    this->text = text;
    this->line = line;
}

AfficheurData::~AfficheurData()
{
    free((void *)id);
    free((void *)text);
    free((void *)line);
}

const char *AfficheurData::get_formatter() const
{
    char *formated = (char *)malloc(sizeof(char) * (strlen(get_line()) + 3 + strlen(get_text()) + 1));
    strcpy(formated, get_line());
    strcat(formated, " - ");
    strcat(formated, get_text());
    return formated;
}


const char *AfficheurData::get_id() const { return id; }
const char *AfficheurData::get_text() const { return text; }
const char *AfficheurData::get_line() const { return line; }