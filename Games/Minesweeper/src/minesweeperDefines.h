#ifndef MINESWEEPERDEFINES_H
#define MINESWEEPERDEFINES_H

#include "precomp.h"
#include <string>
#include <unordered_map>

#define MINESWEEPER_STANDARD_FONT 40
#define MINESWEEPER_CENTER_X sst::baseX/2
#define MINESWEEPER_CENTER_Y sst::baseY/2

#define MINESWEEPER_DEFAULT_TEXTBOX_WIDTH 310
#define MINESWEEPER_DEFAULT_TEXTBOX_HEIGHT MINESWEEPER_STANDARD_FONT + 6
#define MINESWEEPER_TEXT_RIGHT_X sst::baseX - 2
#define MINESWEEPER_TEXT_TOP_Y (float)button_Info.fontSize/2 
#define MINESWEEPER_TEXT_BOTTOM_Y sst::baseY - MINESWEEPER_TEXT_TOP_Y

enum Minesweeper_Menus
{
    MM_Init,
    MM_Host,
    MM_Join,
    MM_CreateServer,
    MM_CreateClient,
    MM_ServerLobby,
    MM_ClientLobby,
    MM_Game
};

struct Minesweeper_Tile_Info
{
    bool is_mine;
    int mine_num;
    rl::Rectangle rec;
    bool revealed;
    bool flagged;
};

enum Minesweeper_Text_Box_Type
{
    MTBT_NONE,
    MTBT_IP,
    MTBT_PORT,
    MTBT_USERNAME
};

struct Minesweeper_Text_Box
{
    rl::Rectangle location;
    float thickness;
    bool isEditing;
    rl::Color color;
    Minesweeper_Text_Box_Type type;
    bool isTextCentered;
};

enum Minesweeper_Location_Orientation
{
    MLO_Center, //Required
    MLO_Top,
    MLO_Left, //Required
    MLO_Right,
    MLO_Bottom
};

struct Minesweeper_Text_Info
{
    std::string text;
    rl::Vector2 textLocationPoint;
    bool isMouseHovering;
    rl::Color selectColor;
    rl::Color nonSelectColor;
    int fontSize;
    bool hasTextBox;
    Minesweeper_Text_Box textBox;
    Minesweeper_Location_Orientation textOrientation;
};

#endif