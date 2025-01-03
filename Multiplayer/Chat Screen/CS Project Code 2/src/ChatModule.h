#ifndef CHATMODULE_H
#define CHATMODULE_H

#include "ChatObject.h"
#include "ChatModuleDefines.h"
#include "networkcodeTCP.h"
#include <string>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <chrono>
#include <deque>
#include <array>
#include <cstring>

class ChatModule
{
    private:
        int program_fps;
        size_t fps_counter;
        bool displayError;
        bool timeToLeaveMenu; //Used for threads
        rl::Rectangle ChatWindow;
        rl::Color WindowColor;
        ChatModule_Menu currentMenu;
        bool allowDrag;
        rl::Vector2 LeftClickPos;


        //Stores buttons using a label and ChatModule_TextInfo
        ChatModule_TextInfo buttonInfo;
        std::unordered_map<std::string, ChatModule_TextInfo> buttons;
        bool areButtonsInitialized;
        //Image
        ChatModule_TextureInfo T_loading;
        std::thread loading_thread;
        std::mutex loading_mtx;
        std::condition_variable loading_cv;
        //Editing text box fields
        ChatModule_EditingTextBox currentlyEditing;
        int usernameLength;
        int messageLength;
        int portLength;
        int ipLength;
        char username[MAX_USERNAME_LEN + 1];
        char message[MAX_MSG_LEN + 1];
        int msgFontSize;
        char port[MAX_PORTNUMBER_LENGTH + 1];
        char ip[MAX_IP_LENGTH + 1];
        bool isTyping;
        std::deque<std::array<char, MAX_USERNAME_LEN + MAX_MSG_LEN + 1>> messageHistory;
        std::mutex messageHistory_mtx;
        void DrawChatHistory();

        //Network
        Network_Error_Msg_Buffer gen_err;
        Network_Server_Locks server_locks;
        Network_Server_Information server_info;
        Network_Client_Locks client_locks;
        Network_Client_Information client_info;
        bool isServer;
        ChatModule_SuperBool wasNetworkCreated;
        std::atomic<bool> isThreadDone;
        std::thread intermediateThread; //Small thread that waits for another thread to be completed
        std::mutex intermediateThread_mtx;
        std::condition_variable intermediateThread_cv;
        void CreateServer();
        void CreateClient();
        std::string errorMsgOut; //To be edited
        void DataHandler(const ChatObject& data);
        size_t client_vec_prev_size; //Used to check if anyone has joined the server

    public:
        ChatModule(int fps);
        ChatModule(int fps, bool isServer, std::string server_ip, std::string server_port, std::string chat_username);
        ~ChatModule();
        void Draw();
        void UpdateState();

    private:
        //Respective menu drawing functions and logic update functions
        void InitializeInit();
        void DrawInit() const;
        void UpdateInit();
        void InitializeSelect();
        void DrawSelect() const;
        void UpdateSelect();
        void InitializeSettings();
        void DrawSettings() const;
        void UpdateSettings();
        void InitializeHost();
        void DrawHost() const;
        void UpdateHost();
        void InitializeJoin();
        void DrawJoin() const;
        void UpdateJoin();

        void DrawLoading() const;
        void UpdateLoading();

        void InitializeServerCreation();
        void UpdateServerCreation();
        void IntermediateServerCreationThread();

        void InitializeChatScreen();
        void DrawChatScreen();
        void UpdateChatScreen();
        void IntermediateServerLeavingThread();
        void IntermediateClientLeavingThread();

        void InitializeClientCreation();
        void UpdateClientCreation();
        void IntermediateClientCreationThread();


        //Drag logic for the menu
        void Drag();

        //Returns the center coordinates for the chatwindow from the perspective of the actual window
        float GetCenterX() const;
        float GetCenterY() const;

        //Rectangle related functions
        const rl::Rectangle CenterRect(int width, int height) const;

        /*
        * Functions that draw text on screen. It is assumed that the mouse can only select one option at a time.
        *
        * Will change info.isMouseHovering to false if the mouse is hovering to ensure following drawings do not also think
        *   that the mouse is hovering on them.
        *
        */
        void DrawTextAboveRect(const ChatModule_TextInfo& info, int padding) const;
        void DrawTextBelowRect(const ChatModule_TextInfo& info, int padding) const;
        void DrawTextLeftOfRect(const ChatModule_TextInfo& info, int padding) const;
        void DrawTextRightOfRect(const ChatModule_TextInfo& info, int padding) const;
        void DrawTextOnPoint(const ChatModule_TextInfo &info) const;
        void DrawTextLeftOfPoint(const ChatModule_TextInfo &info) const;
        void DrawTextCenteredAtXY(const std::string &text, int x, int y, int font, int offsetAbove, rl::Color color) const;
        void DrawRecFromButtonInfo(const ChatModule_TextInfo& info) const;
        void DrawRecLinesFromButtonInfo(const ChatModule_TextInfo& info, float linethickness) const;
        void GetRectFromPoint_Center(const ChatModule_TextInfo &info, rl::Rectangle& rec);
        void GetRectFromPoint_Top(const ChatModule_TextInfo &info, rl::Rectangle& rec);
        void GetRectFromPoint_Left(const ChatModule_TextInfo &info, rl::Rectangle& rec);
        void GetRectFromPoint_Right(const ChatModule_TextInfo &info, rl::Rectangle& rec);
        void GetRectFromPoint_Bottom(const ChatModule_TextInfo &info, rl::Rectangle& rec);

        //Texture Specific
        void InitLoadingImage();

        //Buttons specific
        void ButtonInfoInit();
        void CheckButtonCollision();
        void ModifyTextBox();
        void DrawTextBox(const ChatModule_TextInfo &info, ChatModule_EditingTextBox textboxType, bool isCentered) const;
};



#endif