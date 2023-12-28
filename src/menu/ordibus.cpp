/*
    Imagined design since I did not found any picture of the original Ordibus.
*/

#include "ordibus.hpp"
#include "../layout.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <list>
#include <charconv>
#define BUTTON_WIDTH 80
#define BUTTON_FONT_SIZE 50

static const int ORDIBUS_KEYPADS[12] = {7, 4, 1, 10, 8, 5, 2, 0, 9, 6, 3, 11};
static const char *ORDIBUS_KEYPADS_LABEL[12] = {"7", "4", "1", "V", "8", "5", "2", "0", "9", "6", "3", "←"};

void touch_button(Fl_Widget *widget, void *data)
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
        printf("route id: %s\n", ordibus->get_route_id());
        return;
    }

    ordibus->add_number(button_number);
    // Fl_Button *button = (Fl_Button *)widget;
    // button->labelcolor(FL_DARK_GREEN);
}

Ordibus::Ordibus(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    begin();
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    buttons = std::list<Fl_Button *>();
    Fl_Button *button = nullptr;

    size_t initial_y_offset = 4 * TABS_HEIGHT;
    size_t offset_y = initial_y_offset;
    size_t offset_x = (size_t)((w - BUTTON_WIDTH) * 0.5) - BUTTON_WIDTH;
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
            else if (loc == 11)
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
    if (id < 0 || id > 9)
        return;

    printf("id: %d\n", id);

    route_id.push_back(id);
}

// need to be freed
char *Ordibus::get_route_id() const
{
    char *buffer = new char[route_id.size() + 1];
    int i = 0;
    for (int num : route_id)
    {
        buffer[i++] = '0' + static_cast<char>(num);
    }
    buffer[i] = '\0';
    printf("%s\n", buffer);

    return buffer;
}

void Ordibus::pop_route_id()
{
    if (route_id.size() == 0)
        return;
    route_id.pop_back();
}

void Ordibus::draw()
{
    Fl_Group::draw();
}

Ordibus::~Ordibus()
{
    for (auto button : buttons)
        delete button;
}
