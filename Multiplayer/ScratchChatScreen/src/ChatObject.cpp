#include "ChatObject.h"

ChatObject::ChatObject(){message[0] = '\0'; messageLength = 0; username[0] = '\0'; usernameLength = 0;}

std::string ChatObject::getMessage() const
{
    return message;
}

bool ChatObject::setMessage(const std::string& msg)
{
    if (msg.length() > MAX_MSG_LEN + 1) return false;
    messageLength = msg.length();
    std::copy(msg.begin(), msg.end(), message);
    message[messageLength] = '\0';
    return true;
}

std::string ChatObject::getUsername() const
{
    return username;
}

bool ChatObject::setUsername(const std::string& user)
{
    if (user.length() > MAX_USERNAME_LEN + 1) return false;
    usernameLength = user.length();
    std::copy(user.begin(), user.end(), username);
    username[usernameLength] = '\0';
    return true;
}