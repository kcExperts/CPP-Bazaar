#include <winsock2.h>
#include <ws2tcpip.h> //For InetPton function
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>
#include "chatdata.h"
#include "chatclient.h"

int main(void)
{
    //Variables
    std::string serverIp = "70.26.45.194";
    int port = 20000;
    int allowedConnections = 1;
    ChatData data;
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

    ChatClient client;
    client.connectToServer(serverIp, port);

    for (int i = 0; i < 1; i++){
        std::cout << std::endl << "Enter your message:" << std::endl;
        std::string message;
        std::getline(std::cin, message);
        char messageArray[MAX_MESSAGE_LENGTH];
        std::strcpy(messageArray, message.c_str());
        client.sendMessage(messageArray);
    }
    //Disconnect
    std::cout << std::endl << "Closing Socket..." << std::endl;
    client.disconnect();
    WSACleanup();

    return 0;
}