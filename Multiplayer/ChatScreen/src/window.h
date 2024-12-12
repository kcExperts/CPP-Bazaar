#ifndef WINDOW_H
#define WINDOW_H

#include "precomp.h"
#include "screenSizeTransfer.h"
#include "chatscreen.h"
#include "windowserver.h"
#include "windowclient.h"
#include "windowdata.h"

/*
TODO: Make window dragging not sticky (would have to cancel drag vector in certain direction)
        - Finish updateNetworkLogic
*/

#define CHATWINDOW_HEIGHT 200
#define CHATWINDOW_WIDTH 350
#define CHATWINDOW_ROWS 5
#define CHATWINDOW_TEXT_START_OFFSET 20
#define CHATWINDOW_TEXT_COLOUR {255,255,255, transparency}

#define CHATWINDOW_MESSAGEBOX_X window.x + CHATWINDOW_TEXT_START_OFFSET + 30
#define CHATWINDOW_MESSAGEBOX_Y window.y + window.height - 35
#define CHATWINDOW_MESSAGEBOX_TEXT_Y messageBox.y + 3
#define CHATWINDOW_MAX_MESSAGE_TEXT_SCREENSIZE  400 //Actual allowed screen length of the message //Originally 260
#define CHATWINDOW_MESSAGE_DISPLAY_SIZE_THRESHOLD 260 //Remove if define above is 260
#define CHATWINDOW_MESSAGE_BUFFER_LENGTH 60 //Max characters allowed, but checks CHATWINDOW_MAX_MESSAGE_TEXT_SCREENSIZE as priority

#define CHATWINDOW_USERNAMEBOX_X window.x + window.width/2 - (CHATWINDOW_MAX_USERNAME_TEXT_SCREENSIZE/2)
#define CHATWINDOW_USERNAMEBOX_Y window.y + window.height/4 + 75
#define CHATWINDOW_MAX_USERNAME_TEXT_SCREENSIZE 100
#define CHATWINDOW_USERNAME_BUFFER_LENGTH 8

//To prevent chatscreen from being moved out of the border
struct windowBorders
{
    rl::Vector2 top_left;
    rl::Vector2 bottom_right;
};

class Window
{
    private:
        rl::Rectangle window;
        rl::Rectangle messageBox;
        rl::Rectangle usernameBox;
        unsigned char transparency;
        rl::Vector2 dragOffset;
        int textRows[CHATWINDOW_ROWS];
        ChatScreen chat;
        windowBorders border;

        //Text Related Things
        unsigned int font;
        char message[CHATWINDOW_MESSAGE_BUFFER_LENGTH + 1];
        char username[CHATWINDOW_USERNAME_BUFFER_LENGTH + 1];
        bool isTyping;
        bool isTextTooLong;
        bool isConnected;
        int curMessageSize;
        int curUsernameSize;

        //Server things
        WindowServer server;
        WindowClient client;
        WindowData data;
        WSAData wsaData;
        int wsaerr;
        bool isServer;

    public:
        Window();
        void initClient();
        void initServer();
        void updateLogic();
        void draw();
        void close();

    private:
        void drag();
        void modifyMessage();
        void debugDraw();
        void removeChar();
        void sendMessage();
        void drawChat();
        void drawSelection();
        void sendUsername();
        void modifyUsername();

        void broadcastToAllClients(const std::string& msg);
        void updateNetworkLogic();
        void sendToServer(const std::string& msg);

        int centerTextX(const char* text);
};

#endif