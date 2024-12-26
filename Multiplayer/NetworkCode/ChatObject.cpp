#include "ChatObject.h"

ChatObject::ChatObject(){message[0] = '\0'; messageLength = 0;}

std::string ChatObject::getMessage() const
{
    return (std::string)message;
}

bool ChatObject::setMessage(const std::string& msg)
{
    if (msg.length() > MAX_MSG_LEN) return false;
    messageLength = msg.length();
    std::copy(msg.begin(), msg.end(), message);
    message[messageLength + 1] = '\0';
    return true;
}