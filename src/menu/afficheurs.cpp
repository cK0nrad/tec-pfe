#include "afficheurs.hpp"
#include "../layout.hpp"

#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_BMP_Image.H>

Afficheurs::Afficheurs(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    Fl_Input *input = new Fl_Input(15, 4 * TABS_HEIGHT + 11, (int)(WIDTH * 0.75), TABS_HEIGHT - 2);
    input->box(FL_FLAT_BOX);
    input->color(FL_BLACK);
    input->selection_color(FL_BLACK);
    input->textcolor(FL_WHITE);
    input->textsize(14);
    input->cursor_color(FL_WHITE);

    Fl_Image *loupe_img = new Fl_BMP_Image("./src/assets/loupe.bmp");
    Fl_Image *info_img = new Fl_BMP_Image("./src/assets/info.bmp");

    Fl_Button *info = new Fl_Button(75, 6 * TABS_HEIGHT, 40, 40, "");
    info->box(FL_FLAT_BOX);
    info->image(info_img);
    info->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

    Fl_Button *search = new Fl_Button(300, 6 * TABS_HEIGHT, 40, 40, "");
    search->box(FL_FLAT_BOX);
    search->image(loupe_img);
    search->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

    end();
}

void Afficheurs::draw()
{
    Fl_Group::draw();
    fl_rect(15, 4 * TABS_HEIGHT + 10, (int)(WIDTH * 0.75), TABS_HEIGHT, FL_DARK_BLUE);
    fl_rect(12, 2 * TABS_HEIGHT + 13, (int)(WIDTH * 0.75) + 6, 3 * TABS_HEIGHT, FL_BLACK);

    fl_rectf(15, 2 * TABS_HEIGHT, 70, 20, SECONDARY);
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 10);
    fl_draw("Destination", 16, 2 * TABS_HEIGHT + 4, 70, 20, FL_ALIGN_LEFT);

    fl_font(FL_HELVETICA_BOLD, 16);
    fl_draw("29 - RENTRE AU DEPOT", 12, 2 * TABS_HEIGHT + 13, (int)(WIDTH * 0.75) + 6, 3 * TABS_HEIGHT - 20, FL_ALIGN_CENTER);
}
