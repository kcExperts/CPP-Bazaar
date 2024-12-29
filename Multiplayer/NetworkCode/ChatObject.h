#ifndef CHATOBJECT_H
#define CHATOBJECT_H

#include <string>
#include <algorithm>

constexpr int MAX_MSG_LEN = 100;
constexpr int MAX_USERNAME_LEN = 10;

class ChatObject
{
    private:
        char message[MAX_MSG_LEN + 1];
        char username[MAX_USERNAME_LEN + 1];
        int messageLength;
        int usernameLength;
    public:
        ChatObject();
        std::string getMessage() const;
        bool setMessage(const std::string& msg);
        std::string getUsername() const;
        bool setUsername(const std::string& msg);
};


#endif