#ifndef CHATMODULE_H
#define CHATMODULE_H

#include "ChatObject.h"
#include "ChatModuleDefines.h"
#include <string>
#include <iostream>

class ChatModule
{
    private:
        rl::Rectangle ChatWindow;
        rl::Color WindowColor;
        ChatModule_Menu currentMenu;

        rl::Vector2 LeftClickPos;

        ChatModule_TextureInfo T_loading;


    public:
        ChatModule();
        ~ChatModule();
        void Draw() const;
        void UpdateState();


    private:

        //Respective menu drawing functions and logic update functions
        void DrawInit() const;
        void UpdateInit();
        void DrawSelect() const;
        void UpdateSelect();
        void DrawSettings() const;
        void UpdateSettings();
        void DrawHost() const;
        void UpdateHost();
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
        const rl::Rectangle CenterRect(const rl::Rectangle& rect) const;

        //Drawing text next to rectangles within chatscreen
        void DrawTextAboveRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const;
        void DrawTextBelowRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const;
        void DrawTextLeftOfRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const;
        void DrawTextRightOfRect(const ChatModule_TextInfo& info, const rl::Rectangle& rect, int padding) const;

        //Texture Specific
        void InitLoadingImage();
};



#endif