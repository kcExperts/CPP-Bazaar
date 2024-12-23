#ifndef CHATMODULEDEFINES_H
#define CHATMODULEDEFINES_H

#include "precomp.h"
#include <string>

//Base ChatWindow Dimensions
#define CHATWINDOW_WIDTH 300
#define CHATWINDOW_HEIGHT 200

struct ChatModule_TextInfo
{
    std::string text;
    int font;
    rl::Color color;
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