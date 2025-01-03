#include "chatdata.h"
#include <iostream>

ChatData::ChatData() {}

void ChatData::changeMsgLen(int msglen)
{
    messageLength = msglen;
}

void ChatData::changeUsernameLen(int usernamelen)
{
    usernameLength = usernamelen;
}

bool ChatData::setMessage(const char newMessage[])
{
    if (strlen(newMessage) > MAX_MESSAGE_LENGTH)
        return false;
    std::strcpy(message, newMessage);
    return true;
}

bool ChatData::setUsername(const char newUsername[])
{
    if (strlen(newUsername) > MAX_USERNAME_LENGTH)
        return false;
    std::strcpy(username, newUsername);
    return true;
}

const char* ChatData::getMessage() const
{
    return message;
}

const char* ChatData::getUsername() const
{
    return username;
}