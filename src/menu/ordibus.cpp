/*
    Imagined design since I did not found any picture of the original Ordibus.
*/

#include "ordibus.hpp"
#include "../layout.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <list>
#include <charconv>
#define BUTTON_WIDTH 80
#define BUTTON_FONT_SIZE 50
#define ORDIBUS_SIZE_LIMIT 15
//((WIDTH - BUTTON_WIDTH) * 0.5) - BUTTON_WIDTH
#define CENTER_OFFSET 280

static const int ORDIBUS_KEYPADS[12] = {7, 4, 1, 10, 8, 5, 2, 0, 9, 6, 3, 11};
static const char *ORDIBUS_KEYPADS_LABEL[12] = {"7", "4", "1", "V", "8", "5", "2", "0", "9", "6", "3", "←"};

static void touch_button(Fl_Widget *widget, void *data)
{
    int button_number = *(int *)data;
    Ordibus *ordibus = (Ordibus *)widget->parent();

    if (button_number == 11)
    {
        ordibus->pop_route_id();
        return;
    }
    else if (button_number == 10)
    {
        return;
    }

    ordibus->add_number(button_number);
}

// could have (and should have) extended the class to implement the keyboard
// for any fl_group
Ordibus::Ordibus(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    buttons = std::list<Fl_Button *>();
    route_id = std::list<int>();

    Fl_Button *button = nullptr;

    size_t initial_y_offset = 4 * TABS_HEIGHT;
    size_t offset_y = initial_y_offset;
    size_t offset_x = CENTER_OFFSET;
    size_t loc = 0;
    for (size_t a = 0; a < 3; a++)
    {
        offset_y = initial_y_offset;

        for (size_t b = 0; b < 4; b++)
        {
            button = new Fl_Button(offset_x, offset_y, BUTTON_WIDTH, BUTTON_WIDTH, ORDIBUS_KEYPADS_LABEL[loc]);
            button->callback(touch_button, (void *)&ORDIBUS_KEYPADS[loc++]);
            button->labelsize(BUTTON_FONT_SIZE);
            button->labelfont(FL_BOLD);
            if (loc == 4)
            {
                button->labelcolor(FL_DARK_GREEN);
            }
            else if (loc == 12)
            {
                button->labelcolor(FL_RED);
            }

            buttons.push_back(button);
            offset_y += BUTTON_WIDTH;
        }
        offset_x += BUTTON_WIDTH + 1;
    }

    end();
}

void Ordibus::add_number(int id)
{
    if (route_id.size() >= ORDIBUS_SIZE_LIMIT)
        return;

    if (id < 0 || id > 9)
        return;

    damage(0x90);
    route_id.push_back(id);
}

// need to be freed
char *Ordibus::get_route_id() const
{
    char *buffer = (char *)malloc(sizeof(char) * (route_id.size() + 1));
    int i = 0;
    for (int num : route_id)
    {
        buffer[i++] = '0' + static_cast<char>(num);
    }
    buffer[i] = '\0';

    return buffer;
}

void Ordibus::pop_route_id()
{
    if (route_id.size() == 0)
        return;
    route_id.pop_back();
    damage(0x90);
}

void Ordibus::draw()
{
    if (damage() != 0x90)
    {
        fl_line_style(FL_SOLID, 1);
        Fl_Group::draw();

        fl_font(FL_HELVETICA_BOLD, 20);
        fl_color(FL_WHITE);
        fl_draw("Trip ID:", CENTER_OFFSET - 120, (int)(2.5 * TABS_HEIGHT), 120, TABS_HEIGHT, FL_ALIGN_CENTER);
    }

    fl_line_style(FL_SOLID, 3);
    fl_rectf(CENTER_OFFSET, (int)(2.5 * TABS_HEIGHT), 3 * BUTTON_WIDTH, TABS_HEIGHT, FL_BLACK);

    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA_BOLD, 20);

    char *id = get_route_id();
    fl_draw(id, CENTER_OFFSET, (int)(2.5 * TABS_HEIGHT), 3 * BUTTON_WIDTH, TABS_HEIGHT, FL_ALIGN_CENTER);
    free((void *)id);
}

Ordibus::~Ordibus()
{
    for (auto button : buttons)
        delete button;
}
