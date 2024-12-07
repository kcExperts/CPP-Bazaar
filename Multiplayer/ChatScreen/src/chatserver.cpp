#include "chatserver.h"

ChatServer::ChatServer()
{
    running = false;
    totalConnections = 0;
}

bool ChatServer::create(std::string ip, int port, int maxConnections, int recvBuffer)
{
    //Create the socket

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        std::cout << "Socket Creation Failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    //Bind the socket

    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(port);
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::cout << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server);
        return false;
    }

    //Perform a deep copy of the data
    this->ip = ip;
    this->port = port;
    this->maxConnections = maxConnections;
    this->maxAllowedChars = recvBuffer;
    return true;
}

void ChatServer::listenForClients()
{
    running = true;
    listenThread = std::thread([this]() {
        //Listen
        if (listen(server, maxConnections) == SOCKET_ERROR)
        {
            std::cerr << "Listening Failed: " << WSAGetLastError() << std::endl;
            stopThread();
            return;
        }

        //Accept new client
        while (running)
        {
            SOCKET potentialClient = accept(server, NULL, NULL);
            if (potentialClient == INVALID_SOCKET)
            {
                std::cerr << "Accept Failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            //Store the client and assign a thread to it
            serverConnections.push_back(potentialClient);
            int clientID = totalConnections;
            std::thread(&ChatServer::recvFromClient, this, potentialClient, clientID).detach(); //Might need to use serverConnections here
            totalConnections++;
        }
        std::cout << "Server has stopped Listening for New Connections." << std::endl;
    });
}

void ChatServer::recvFromClient(SOCKET client, int clientID)
{   
    std::cout << "Thread handling client: " << clientID << " started." << std::endl;
    char buffer[maxAllowedChars];
    int bytesReceived;
    while(true)
    {
        //Store message and check for errors
        bytesReceived = recv(client, buffer, maxAllowedChars, 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            std::cerr << "Client " << clientID << " received failed: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0)
        {
            std::cerr << "Client " << clientID << " has disconnected" << std::endl;
            break;
        }
        std::cout << "Client " << clientID << ": " << buffer << std::endl;
    }
    shutdown(client, SD_BOTH);
    closesocket(client);
    std::cout << "Connection with client " << clientID << " terminated." << std::endl; 
}

void ChatServer::stopThread()
{
    running = false;
    //If the thread is still running, wait for it to finish before stopping
    if (listenThread.joinable()) 
        listenThread.join();
}

void ChatServer::stopListening()
{
    stopThread();
}

void ChatServer::shutItDown()
{
    stopThread(); //PROBLEMS
    shutdown(server, SD_BOTH);
    closesocket(server);
    std::cout << "Server succesfully terminated." << std::endl;
}
