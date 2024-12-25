#ifndef CHATMODULEDEFINES_H
#define CHATMODULEDEFINES_H

#include "precomp.h"
#include <string>

#define STANDARD_FONT_SIZE 19

//Base ChatWindow Dimensions
#define CHATWINDOW_WIDTH 300
#define CHATWINDOW_HEIGHT 200
#define CHATWINDOW_START_X sst::baseX/2
#define CHATWINDOW_START_Y sst::baseY/2

//Username box dimensions
#define CHATWINDOW_USERNAMEBOX_WIDTH 150
#define CHATWINDOW_USERNAMEBOX_HEIGHT STANDARD_FONT_SIZE + 6

//Chatwindow text offset
#define CHATWINDOW_OFFSET_X (ChatWindow.x - CHATWINDOW_START_X)
#define CHATWINDOW_OFFSET_Y (ChatWindow.y - CHATWINDOW_START_Y)

//Chat length
constexpr size_t MAX_USERNAME_LENGTH = 10;
constexpr size_t MAX_MESSAGE_LENGTH = 200;
constexpr size_t MAX_MESSAGE_HISTORY_STORAGE_SIZE = 5;
constexpr size_t MAX_PORTNUMBER_LENGTH = 5;
constexpr size_t MAX_IP_LENGTH = 16;

//Other
#define CHATWINDOW_TOPRIGHT_X ChatWindow.x + ChatWindow.width - rl::MeasureText(buttonInfo.text.c_str(), buttonInfo.font)
#define CHATWINDOW_TOPRIGHT_Y ChatWindow.y + buttonInfo.font/2
#define CHATWINDOW_BOTTOMRIGHT_Y ChatWindow.y + ChatWindow.height - buttonInfo.font/2


//Lists all possible text edit fields
enum ChatModule_EditingTextBox
{
    None,
    Username,
    Port,
    Ip,
    Message
};

struct ChatModule_TextBox
{
    rl::Rectangle location;
    float thickness;
    bool isEditing;
    rl::Color color;
    ChatModule_EditingTextBox type;

};

struct ChatModule_TextInfo
{
    std::string text;
    int font;
    rl::Color nonSelectColor;
    rl::Color selectColor;
    bool isMouseHovering = false;
    rl::Vector2 textLocationPoint;
    //Only used for text boxes
    ChatModule_TextBox textBox;
    bool hasTextBox = false;
};

//Meant to be used with raylib's DrawTexturePro function
struct ChatModule_TextureInfo
{
    rl::Texture2D texture;
    rl::Rectangle sourceRec;
    rl::Vector2 textureCenter;
    float currentAngle;
    int animationStages;
    int currentAnimationStage;
    int framesUntilUpdate;
    int framesCounted;
};

//Lists all the menus for the chatmodule
enum ChatModule_Menu
{
    Init,
    Select,
    Settings,
    Host,
    Join,
    Loading
};




#endif