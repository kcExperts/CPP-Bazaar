#include "windowdata.h"

#include <cstring>

WindowData::WindowData() {};

void WindowData::setMessage(const char message[])
{
    strncpy(socketMessage, message, WINDOWDATA_MAX_CHAR_LEN);
    socketMessage[WINDOWDATA_MAX_CHAR_LEN - 1] = '\0';
}

const char* WindowData::getMessage() const
{
    return socketMessage;
}