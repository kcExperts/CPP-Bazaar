#include "windowserver.h"

WindowServer::WindowServer()
{
    isListenRunning = false;
    failure = false;
    ID = 0;
    totalConnections = 0;
}

WindowServer::~WindowServer()
{
    chat = nullptr;
    shutDown();
}

bool WindowServer::create(std::string ip, int port, int maxConnections, ChatScreen& chatRef)
{
    //Socket creation
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET) return false;

    //Binding
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un);
    service.sin_port = htons(port);
    if (bind(server, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) return false;
    
    //Save relevant info
    this->ip = ip;
    this->port = port;
    this->maxConnections = maxConnections;
    this->chat = &chatRef;
    return true;
}

void WindowServer::listenForClients()
{
    isListenRunning = true;
    listenThread = std::thread([this]() {
        //Listen
        if (listen(server, maxConnections) == SOCKET_ERROR) failure = true;

        //Accept new client
        while (isListenRunning)
        {
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(server, &readSet);
            timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int activity = select(0, &readSet, NULL, NULL, &timeout);
            if (activity == SOCKET_ERROR) failure = true;

            if (activity > 0 && FD_ISSET(server, &readSet)) {
                SOCKET potentialClient = accept(server, NULL, NULL);
                if (potentialClient == INVALID_SOCKET) failure = true;

                // Process new connection
                totalConnections++;
                ID++;
                clientConnectionFlags[ID] = std::make_pair(
                    std::make_unique<std::atomic<bool>>(true),
                    potentialClient
                );
                //Start a receive thread
                std::thread(&WindowServer::receiveFromClient, this, ID).detach();
                totalConnections++;
            }
        }

    });
}

void WindowServer::broadcast(WindowData& data)
{
    for (auto& [clientID, pair] : clientConnectionFlags)
    {
        SOCKET clientSocket = pair.second;
        int result = send(clientSocket, (char*)&data, sizeof(data), 0);
        //No error check, assume message went through
    }
}

bool WindowServer::isWorking() {return !failure;}

void WindowServer::shutDown()
{
    isListenRunning = false;
    //If the thread is still running, wait for it to finish before stopping
    if (listenThread.joinable()) 
        listenThread.join();
    for (auto& [clientID, pair] : clientConnectionFlags)
    {
        if (clientConnectionFlags.find(clientID) != clientConnectionFlags.end())
        {
        *clientConnectionFlags[clientID].first = false;
        //Shutdown socket
        SOCKET clientSocket = clientConnectionFlags[clientID].second;
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        //Erase from map
        clientConnectionFlags.erase(clientID);
        }
    }
}

void WindowServer::receiveFromClient(int ID)
{
    // Retrieve socket and running flag
    auto& entry = clientConnectionFlags[ID];
    auto& runningFlag = *entry.first; // Dereference unique_ptr to access atomic<bool>
    SOCKET client = entry.second;
    WindowData recvData;
    int bytesReceived;

    while (true)
    {
        //Store message and check for errors
        bytesReceived = recv(client, (char*)&recvData, sizeof(recvData), 0);
        if (bytesReceived == SOCKET_ERROR) break; //Failure
        if (bytesReceived == 0) break; //Disconnected
        chat->addMessage(recvData.getMessage());
    }

    if (clientConnectionFlags.find(ID) != clientConnectionFlags.end())
    {
        *clientConnectionFlags[ID].first = false; // Set running flag to false
        SOCKET clientSocket = clientConnectionFlags[ID].second;
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);
        // Remove the client from the map
        clientConnectionFlags.erase(ID);
    }
}

