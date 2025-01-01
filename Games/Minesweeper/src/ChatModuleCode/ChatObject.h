#ifndef CHATOBJECT_H
#define CHATOBJECT_H

#include <string>
#include <algorithm>
#include "../minesweeperDefines.h"

constexpr int MAX_MSG_LEN = 100;
constexpr int MAX_USERNAME_LEN = 10;

class ChatObject
{
    private:
        char message[MAX_MSG_LEN + 1];
        char username[MAX_USERNAME_LEN + 1];
        int messageLength;
        int usernameLength;

        Minesweeper_Tile_Info map[529];
        int map_size;
        int menu;
        int tileSelected;
        bool isDead;
        bool didWin;
        bool isMapInfo;

    public:
        ChatObject();
        std::string getMessage() const;
        bool setMessage(const std::string& msg);
        std::string getUsername() const;
        bool setUsername(const std::string& msg);
        void setGameMenu(int curMenu);
        int getGameMenu();
        void setMapInfo(const Minesweeper_Tile_Info& tile, int tileNumber);
        const Minesweeper_Tile_Info& getMapInfo(int tileNumber);
        void setTileSelected(int tile);
        int getTileSelected();
        void setIsDead(bool b);
        void setDidWin(bool b);
        bool getIsDead();
        bool getDidWin();
        void setMapSize(int mapSize);
        int getMapSize();
        void setIsMap(bool b);
        bool getIsMap();
};


#endif