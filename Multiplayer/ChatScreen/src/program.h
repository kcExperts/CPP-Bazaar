#include "precomp.h"
#include "chatdata.h"
#include "chatclient.h"
#include "chatscreen.h"
#include "chatserver.h"
#include "constants.h"
#include "screenSizeTransfer.h"
#include "mouse.h"
#include "button.h"
#include <vector>
#include <memory>


class Program
{
    private:
        //Network
        WSAData wsadata;
        std::unique_ptr<ChatServer> server;
        std::unique_ptr<ChatClient> client;
        ChatScreen screen;
        //For Raylib
        Mouse mouse;
        std::vector<Button> buttons;
        
        bool isServer;
        bool isClient;
        CurrentMenu menu;
        bool end;
        bool isMenuInitialized;
        bool debug;
    public:
        Program();
        void loop();
        void close();
    private:
        void updateLogic(ChatEvents state);
        ChatEvents getStateTransition();
        void initializeMenu();
        void draw();
        void drawDebug();

        void drawMainMenu();
        void drawClientJoinMenu();
        void drawHostSettingsMenu();
        void drawHostMain();
        void drawClientMain();

        ChatEvents getMainMenuState();
        ChatEvents getClientJoinMenuState();
        ChatEvents getHostSettingsMenuState();
        ChatEvents getHostMainState();
        ChatEvents getClientMainState();

        
        //Utility functions
        //Adds a button based off text
        void addButton(const std::string& label, int font, int posX, int posY);
        //Adds a button based off a rectangle
        void addButton(const std::string& label, rl::Rectangle rec);
        //Returns the x position that centers the text on the screen (no matter the screen dimensions)
        int centerTextX(const std::string& text, int font);
        //Returns the x position that centers the text on the screen (no matter the screen dimensions)
        int centerTextY(const std::string& text, int font);
        //Returns an int to the button being hovered
        int buttonHovered() const;


};