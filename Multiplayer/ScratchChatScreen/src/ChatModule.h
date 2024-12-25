#ifndef CHATMODULE_H
#define CHATMODULE_H

#include "ChatObject.h"
#include "ChatModuleDefines.h"
#include <string>
#include <iostream>
#include <unordered_map>

class ChatModule
{
    private:
        rl::Rectangle ChatWindow;
        rl::Color WindowColor;
        ChatModule_Menu currentMenu;

        rl::Vector2 LeftClickPos;

        //Stores buttons using a label and ChatModule_TextInfo
        ChatModule_TextInfo buttonInfo;
        std::unordered_map<std::string, ChatModule_TextInfo> buttons;
        bool areButtonsInitialized;
        //Image
        ChatModule_TextureInfo T_loading;
        //Editing text box fields
        ChatModule_EditingTextBox currentlyEditing;
        int usernameLength;
        int messageLength;
        int portLength;
        int ipLength;
        char username[MAX_USERNAME_LENGTH + 1];
        char message[MAX_MESSAGE_LENGTH + 1];
        char port[MAX_PORTNUMBER_LENGTH + 1];
        char ip[MAX_IP_LENGTH + 1];
        bool isTyping;

    public:
        ChatModule();
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
        void DrawTextLeftOfPoint(const ChatModule_TextInfo &info, int padding) const;
        void GetRectFromPoint(const ChatModule_TextInfo &info, rl::Rectangle& rec);

        //Texture Specific
        void InitLoadingImage();

        //Buttons specific
        void ButtonInfoInit();
        void CheckButtonCollision();
        void ModifyTextBox();
        void DrawTextBox(const ChatModule_TextInfo &info, ChatModule_EditingTextBox textboxType) const;
};



#endif