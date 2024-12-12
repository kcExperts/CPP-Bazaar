#ifndef CHATSCREEN_H
#define CHATSCREEN_H

#include <deque>
#include <mutex>
#include <iostream>
#include <cstring>
#include <array>
#include "constants.h"

class ChatScreen
{
    private:
        std::deque<std::array<char, MAX_MESSAGE_LENGTH + 1>> messageHistory;
        std::mutex mtx;
        void displayHistory();
    public:
        ChatScreen();
        void addMessage(const char* message);
        const char* getMessage(int index) const;
};

#endif