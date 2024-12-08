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
    InetPtonA(AF_INET, ip.c_str(), &service.sin_addr.S_un); //Do not use InetPton as it defaults to InetPtonW meaning we need widestr
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
            int clientID = totalConnections;
            clientConnectionFlags[clientID] = std::make_pair(
                std::make_unique<std::atomic<bool>>(true), // Running flag
                potentialClient                            // Socket
            );
            std::thread(&ChatServer::recvFromClient, this, clientID).detach();
            totalConnections++;
        }
        std::cout << "Server has stopped Listening for New Connections." << std::endl;
    });
}

void ChatServer::recvFromClient(int clientID)
{   
    // Retrieve socket and running flag
    auto& entry = clientConnectionFlags[clientID];
    auto& runningFlag = *entry.first; // Dereference unique_ptr to access atomic<bool>
    SOCKET client = entry.second;

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
            std::cout << "Client " << clientID << " has disconnected" << std::endl;
            break;
        }
        std::cout << "Client " << clientID << ": " << buffer << std::endl;
    }
    stopClient(clientID); //Stop thread
    closesocket(client);
    clientConnectionFlags.erase(clientID);
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

void ChatServer::stopClient(int clientID)
{
    if (clientConnectionFlags.find(clientID) != clientConnectionFlags.end())
    {
        *clientConnectionFlags[clientID].first = false; // Set running flag to false

        // Optionally close the socket to force termination
        SOCKET clientSocket = clientConnectionFlags[clientID].second;
        shutdown(clientSocket, SD_BOTH);
        closesocket(clientSocket);

        // Remove the client from the map
        clientConnectionFlags.erase(clientID);
    }
}

void ChatServer::sendToClient(int clientID, const std::string& message)
{
    if (clientConnectionFlags.find(clientID) != clientConnectionFlags.end())
    {
        SOCKET clientSocket = clientConnectionFlags[clientID].second;

        // Send the message
        int result = send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR)
        {
            std::cerr << "Failed to send message to client " << clientID << ": " << WSAGetLastError() << std::endl;
        }
        else
        {
            std::cout << "Message sent to client " << clientID << ": " << message << std::endl;
        }
    }
    else
    {
        std::cerr << "Client " << clientID << " not found or disconnected." << std::endl;
    }
}

void ChatServer::broadcast(const std::string& message)
{
    for (auto& [clientID, pair] : clientConnectionFlags)
    {
        SOCKET clientSocket = pair.second;

        // Send the message to the current client
        int result = send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        if (result == SOCKET_ERROR)
        {
            std::cerr << "Failed to send message to client " << clientID << ": " << WSAGetLastError() << std::endl;
        }
        else
        {
            std::cout << "Message broadcasted to client " << clientID << ": " << message << std::endl;
        }
    }
}



