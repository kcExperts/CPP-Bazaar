#include "chatserver.h"

int main(void)
{
    int test;
    //Initialize WSA

    WSAData wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2,2); //Want version 2.2 of winsock2, which is the most recent one
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0)
    {
        std::cout << "Winsock not found" << std::endl;
        return 0;
    }


    ChatServer server;
    if (!server.create("0.0.0.0", 20000, 2, 200))
    {
        std::cout << "Creation Failed" << std::endl;
        return 0;
    }
    server.listenForClients();

    std::cout << "Waiting for Input." << std::endl;
    std::string message;
    std::getline(std::cin, message);
    char messageArray[MAX_MESSAGE_LENGTH];
    std::strcpy(messageArray, message.c_str());
    server.sendToClient(0, messageArray);
    //Make code
    std::cout << "Waiting for Input2." << std::endl;
    std::getline(std::cin, message);

    server.shutItDown();
    return 0;
}