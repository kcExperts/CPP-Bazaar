#ifndef CHATDATA_H
#define CHATDATA_H

#include <string>
#include <sstream>
#include <cstring>
#include "constants.h"

/*
Data object for the chat server. 
It is assumed that only integers > 0 are passed to the max
messageLength and max usernameLength
*/

class ChatData
{
    private:
        int usernameLength;
        int messageLength;
        char username[MAX_USERNAME_LENGTH + 1] = "\0";
        char message[MAX_MESSAGE_LENGTH + 1] = "\0";
    public:
        ChatData();
        void changeMsgLen(int msglen);
        void changeUsernameLen(int usernamelen);
        bool setMessage(const char newMessage[]);
        bool setUsername(const char newUsername[]);
        const char* getMessage() const;
        const char* getUsername() const;
};

#endif