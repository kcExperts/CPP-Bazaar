#include <winsock2.h>
#include <ws2tcpip.h> //For InetPton function
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>

int main(void)
{
    //Variables
    std::string serverIp = "127.0.0.1";
    int port = 55555;
    int allowedConnections = 1;
    char buffer[200];

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

    std::cout << "WSA Initialized Succesfully" << std::endl;

    //Create Socket

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create a socket that uses TCP protocol
    if (clientSocket == INVALID_SOCKET)
    {
        std::cout << "Error: " << WSAGetLastError() << std::endl;
        WSACleanup(); //Cleaup the winsock2 dll file
        return 0;
    }

    std::cout << "Socket Created Succesfully" << std::endl;

    //Connect to the server

    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    InetPton(AF_INET, serverIp.c_str(), &clientService.sin_addr.S_un);
    clientService.sin_port = htons(port);
    if (connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
    {
        std::cout << "Failed to connect: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
    }

    std::cout << "Succesfully Connected to Server" << std::endl;

    //Send and receive data
    for (int i = 0; i < 2; i++){
        std::cout << std::endl << "Enter your message:" << std::endl;
        std::cin.getline(buffer, 200); 
        int byteCount = send(clientSocket, buffer, 200, 0); //No flags needed
        if (byteCount == SOCKET_ERROR)
        {
            std::cout << "Error in sending Message: " << WSAGetLastError() << std::endl;
            return 0;
        }
        std::cout << "Message Received" << std::endl;
    }
    //Disconnect
    std::cout << std::endl << "Closing Socket..." << std::endl;
    shutdown(clientSocket, SD_BOTH);
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}