#ifndef MINESWEEPER_H
#define MINESWEEPER_H

#include "minesweeperDefines.h"
#include "precomp.h"
#include <vector>
#include "ChatModuleCode\ChatModule.h"
#include <memory>
#include <cstdlib> //Rand

class Minesweeper
{
    private:
        int program_fps;
        int map_size;
        std::vector<Minesweeper_Tile_Info> map;
        int total_mines;
        bool isDead;
        bool didWin;
        std::unordered_map<std::string, Minesweeper_Text_Info> buttons;
        Minesweeper_Menus current_Menu;
        bool isMenuInitialized;
        Minesweeper_Text_Info button_Info;
        int difficulty;
        int tileSelected;
        bool mouseInGame;
        int mineNumFontSize;

        int usernameLength;
        int portLength;
        int ipLength;
        char username[MAX_USERNAME_LEN + 1];
        char port[MAX_PORTNUMBER_LENGTH + 1];
        char ip[MAX_IP_LENGTH + 1];
        bool isTyping;
        std::unique_ptr<ChatModule> chatModule;
        
    public:
        bool debug_viewer;
        Minesweeper(int fps);
        ~Minesweeper();
        void UpdateState();
        void Draw();
    
    private:
        void Update_Difficulty();
        void Generate_Map();
        //Does check win condition
        void Draw_Map();
        void Delete_Map();
        void Reveal_Tile(int tile);
        rl::CLITERAL(Color) Generate_Color(int mine_number);

        void Initialize_Init();
        void Draw_Init();
        void Update_Init();

        void Initialize_Host();
        void Draw_Host();
        void Update_Host();

        void Initialize_Join();
        void Draw_Join();
        void Update_Join();

        void Initialize_CreateServer();
        void Draw_CreateServer();
        void Update_CreateServer();

        void Initialize_CreateClient();
        void Draw_CreateClient();
        void Update_CreateClient();
        
        void Initialize_ServerLobby();
        void Draw_ServerLobby();
        void Update_ServerLobby();

        void Initialize_ClientLobby();
        void Draw_ClientLobby();
        void Update_ClientLobby();

        void Initialize_Game();
        void Draw_Game();
        void Update_Game();

        //Utility functions to make my life easier
        void DrawTextOnPoint(const Minesweeper_Text_Info &info) const;
        void DrawTextLeftOfPoint(const Minesweeper_Text_Info &info) const;
        void CheckButtonCollision();
        void GetRectFromPoint(const Minesweeper_Text_Info &info, rl::Rectangle &rec);
        void DrawTextAboveRect(const Minesweeper_Text_Info &info, int padding) const;
        void ModifyTextBox();
        void BasicInitialize();
        const rl::Rectangle CenterRect(int width, int height) const;

};

#endif