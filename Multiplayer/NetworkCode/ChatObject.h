#ifndef CHATOBJECT_H
#define CHATOBJECT_H

#include <string>
#include <algorithm>

constexpr int MAX_MSG_LEN = 100;

class ChatObject
{
    private:
        char message[MAX_MSG_LEN + 1];
        int messageLength;
    public:
        ChatObject();
        std::string getMessage() const;
        bool setMessage(const std::string& msg);
};


#endif