#include "billetique.hpp"
#include "../layout.hpp"

#include <FL/Fl_Button.H>
#include <FL/fl_draw.H>

#define BUTTON_WIDTH 80

// Left, mid and right group
#define LT_ROW 4
#define LT_COL 2

#define MT_ROW 4
#define MT_COL 3

#define RT_ROW 4
#define RT_COL 3

#define SECOND_TAB_LENGTH 12
#define THIRD_TAB_LENGTH 12

// 4*BUTTON_WIDTH + 3
#define DEFAULT_OFFSET 50

const char *KEYS[] = {
    "ON\nOFF",
    "V",
    "X",
    "AR",
    "F",
    "L",
    "R",
    "Z",
    //
    "7",
    "4",
    "1",
    "0",
    "8",
    "5",
    "2",
    "CLR",
    "9",
    "6",
    "3",
    "←",
    //
    "UTIL\nIMM",
    "TARIF\nSPEC",
    "MULTI",
    "M",
    "HOR.+",
    "NEXT",
    "HORIZ",
    "+",
    "X",
    "S",
    "*",
    "P"};
const Fl_Color KEYS_COLOR[] = {
    FL_BLACK,
    FL_BLACK,
    FL_RED,
    FL_BLACK,
    FL_RED,
    FL_BLACK,
    FL_MAGENTA,
    FL_BLUE,

    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,

    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
    FL_BLACK,
};

const Fl_Color KEYS_SIZE[] = {
    20,
    50,
    50,
    50,
    50,
    50,
    50,
    50,

    50,
    50,
    50,
    50,
    50,
    50,
    50,
    24,
    50,
    50,
    50,
    50,

    16,
    16,
    16,
    50,
    16,
    16,
    16,
    50,
    50,
    50,
    50,
    50,
};

// ugly hack to make a container with a cute border
class LessWhityContainer : public Fl_Group
{
public:
    LessWhityContainer(int X, int Y, int W, int H, const char *L = 0)
        : Fl_Group(X, Y, W, H, L)
    {
        box(FL_FLAT_BOX);
    }

    void draw() override
    {
        fl_color(SECONDARY_LIGHT);
        fl_line(x() + w() - 2, y() + 2, x() + w() - 2, y() + h() - 2);
        fl_line(x() + 2, y() + h() - 2, x() + w() - 2, y() + h() - 2);
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 24);
        fl_draw((char *)this->user_data(), x() + 2, y() + h() - 5);
        fl_color(FL_BLACK);
        fl_line(x() + 2, y() + 2, x() + w() - 2, y() + 2);
        fl_line(x() + 2, y() + 2, x() + 2, y() + h() - 2);
    }
};

// No clue on how this should work so no logic behind buttons
Billetique::Billetique(int x, int y, int w, int h, const char *l)
    : Fl_Group(x, y, w, h, l)
{
    begin();
    // bg (could be wrapped before the module but I'm lazy)
    Fl_Group *menu = new Fl_Group(0, 2 * TABS_HEIGHT, WIDTH, HEIGHT - 2 * TABS_HEIGHT);
    menu->box(FL_FLAT_BOX);
    menu->color(SECONDARY);
    menu->end();

    // col
    LessWhityContainer *col_name = new LessWhityContainer(0, 2 * TABS_HEIGHT, WIDTH, TABS_HEIGHT);
    col_name->box(FL_DOWN_BOX);
    col_name->color(SECONDARY);
    const char *text = "LIGNE DIR. REG. ZONE   VOY.     ODM";
    col_name->user_data((void *)text);
    col_name->labelcolor(FL_WHITE);
    col_name->end();

    LessWhityContainer *col_val = new LessWhityContainer(0, 3 * TABS_HEIGHT, WIDTH, TABS_HEIGHT);
    col_val->box(FL_DOWN_BOX);
    col_val->color(SECONDARY);
    const char *text2 = "3820     R     45     01     0028     0412";
    col_val->user_data((void *)text2);
    col_val->end();

    size_t loc = 0;
    buttons = (Fl_Button **)malloc(sizeof(Fl_Button) * (8 + 12 + 12));
    Fl_Button *current_button = nullptr;
    /*
        This is indeed a bad flow.
        Should be wrapped around for loop and propper build method
    */

    size_t i = 0;

    size_t y_pos_init = 4 * TABS_HEIGHT;
    size_t y_pos = y_pos_init;

    size_t x_pos = size_t(WIDTH * 0.5);
    x_pos -= 4 * BUTTON_WIDTH + 3 + (size_t)(BUTTON_WIDTH / 2);

    for (size_t a = 0; a < LT_COL; a++)
    {
        for (size_t b = 0; b < LT_ROW; b++)
        {
            current_button = new Fl_Button(x_pos, y_pos, BUTTON_WIDTH, BUTTON_WIDTH);
            current_button->label(KEYS[i++]);
            current_button->labelsize(KEYS_SIZE[loc]);
            current_button->labelfont(FL_BOLD);
            current_button->labelcolor(KEYS_COLOR[loc]);
            buttons[loc++] = current_button;
            y_pos += BUTTON_WIDTH;
        }
        y_pos = y_pos_init;
        x_pos += BUTTON_WIDTH + 1;
    }
    y_pos = y_pos_init;
    x_pos += size_t(BUTTON_WIDTH / 2);

    for (size_t a = 0; a < MT_COL; a++)
    {
        for (size_t b = 0; b < MT_ROW; b++)
        {
            current_button = new Fl_Button(x_pos, y_pos, BUTTON_WIDTH, BUTTON_WIDTH);
            current_button->label(KEYS[i++]);
            current_button->labelsize(KEYS_SIZE[loc]);
            current_button->labelfont(FL_BOLD);
            current_button->labelcolor(KEYS_COLOR[loc]);
            buttons[loc++] = current_button;
            y_pos += BUTTON_WIDTH;
        }
        y_pos = y_pos_init;
        x_pos += BUTTON_WIDTH + 1;
    }
    printf("x_pos: %ld\n", x_pos);

    y_pos = y_pos_init;
    x_pos += size_t(BUTTON_WIDTH / 2);

    for (size_t a = 0; a < RT_COL; a++)
    {
        for (size_t b = 0; b < RT_ROW; b++)
        {
            current_button = new Fl_Button(x_pos, y_pos, BUTTON_WIDTH, BUTTON_WIDTH);
            current_button->label(KEYS[i++]);
            current_button->labelsize(KEYS_SIZE[loc]);
            current_button->labelfont(FL_BOLD);
            current_button->labelcolor(KEYS_COLOR[loc]);
            buttons[loc++] = current_button;
            y_pos += BUTTON_WIDTH;
        }
        y_pos = y_pos_init;
        x_pos += BUTTON_WIDTH + 1;
    }
    end();
}

void Billetique::draw()
{
    Fl_Group::draw_children();
    Fl_Group::draw();
}
