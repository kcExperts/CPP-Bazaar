#include <winsock2.h>
#include <ws2tcpip.h> //For InetPton function
#include <tchar.h> //For _T macro
#include <stdio.h>
#include <iostream>
#include <string>

int main(void)
{
    //Variables
    std::string ip = "127.0.0.1";
    int port = 55555;
    int allowedConnections = 1;
    char recvBuffer[200];

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

    //Create Socket

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //Create a socket that uses TCP protocol
    if (serverSocket == INVALID_SOCKET)
    {
        std::cout << "Error: " << WSAGetLastError() << std::endl;
        WSACleanup(); //Cleaup the winsock2 dll file
        return 0;
    }

    //Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(port);
    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::cout << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }


    //Listen on the socket

    if (listen(serverSocket, allowedConnections) == SOCKET_ERROR)
        std::cout << "Error: Socket not listening." << std::endl;

    //Accept a connection

    SOCKET acceptSocket;
    acceptSocket = accept(serverSocket, NULL, NULL); //Null means that we do not care about getting info about client socket
    if (acceptSocket == INVALID_SOCKET)
    {
        std::cout << "Accept() Failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }

    //Send and receive data
    int byteCount = recv(acceptSocket, recvBuffer, 200, 0); //No flags needed
    if (byteCount < 0)
    {
        std::cout << "Error in getting Message: " << WSAGetLastError() << std::endl;
        return 0;
    }

    std::cout << recvBuffer;

    //Disconnect
    std::cout << std::endl << "Closing Socket..." << std::endl;
    shutdown(serverSocket, SD_BOTH);
    closesocket(serverSocket);
    WSACleanup();
}
