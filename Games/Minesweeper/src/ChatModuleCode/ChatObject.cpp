#include "ChatObject.h"

ChatObject::ChatObject(){message[0] = '\0'; messageLength = 0; username[0] = '\0'; usernameLength = 0; int menu = 0; isMapInfo = false;}

std::string ChatObject::getMessage() const
{
    return message;
}

bool ChatObject::setMessage(const std::string& msg)
{
    if (msg.length() > MAX_MSG_LEN + 1) return false;
    messageLength = msg.length();
    std::copy(msg.begin(), msg.end(), message);
    message[messageLength] = '\0';
    return true;
}

std::string ChatObject::getUsername() const
{
    return username;
}

bool ChatObject::setUsername(const std::string& user)
{
    if (user.length() > MAX_USERNAME_LEN + 1) return false;
    usernameLength = user.length();
    std::copy(user.begin(), user.end(), username);
    username[usernameLength] = '\0';
    return true;
}

void ChatObject::setGameMenu(int curMenu)
{
    menu = curMenu;
}

int ChatObject::getGameMenu()
{
    return menu;
}

void ChatObject::setMapInfo(const Minesweeper_Tile_Info& tile, int tileNumber)
{
    map[tileNumber] = tile;
}

const Minesweeper_Tile_Info& ChatObject::getMapInfo(int tileNumber)
{
    return map[tileNumber];
}

void ChatObject::setTileSelected(int tile)
{
    tileSelected = tile;
}

int ChatObject::getTileSelected()
{
    return tileSelected;
}

void ChatObject::setIsDead(bool b)
{
    isDead = b;
}

void ChatObject::setDidWin(bool b)
{
    didWin = b;
}

bool ChatObject::getIsDead()
{
    return isDead;
}

bool ChatObject::getDidWin()
{
    return didWin;
}

void ChatObject::setMapSize(int mapSize)
{
    map_size = mapSize;
}

int ChatObject::getMapSize()
{
    return map_size;
}

void ChatObject::setIsMap(bool b)
{
    isMapInfo = b;
}

bool ChatObject::getIsMap()
{
    return isMapInfo;
}