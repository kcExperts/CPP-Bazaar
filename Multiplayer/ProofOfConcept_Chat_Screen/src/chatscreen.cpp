#include "chatscreen.h"

ChatScreen::ChatScreen() {}

void ChatScreen::addMessage(const char* message) {
    std::lock_guard<std::mutex> lock(mtx);

    std::array<char, MAX_MESSAGE_LENGTH + 1> storedMessage;  //Use std::array
    std::strncpy(storedMessage.data(), message, MAX_MESSAGE_LENGTH);
    storedMessage[MAX_MESSAGE_LENGTH] = '\0'; //Ensure null termination

    messageHistory.push_back(storedMessage);
    if (messageHistory.size() > MAX_MESSAGE_HISTORY_STORAGE_SIZE) {
        messageHistory.pop_front(); //Remove oldest message
    }
}

const char* ChatScreen::getMessage(int index) const
{
    if (index >= messageHistory.size()) return "";
    return messageHistory.at(index).begin();
}

void ChatScreen::displayHistory()
{
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& message : messageHistory)
    {
        //Do whatever with messages
        std::cout << message.data() << std::endl;  //Access array content using .data()
    }
}
